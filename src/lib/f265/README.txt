Haswell optimization.

* Optimizing for Haswell:

This section provides supplementary information for optimizing the assembly
code on Haswell. The main documentation is contained in the links provided
below. Read that first. Take your time. You need a rather deep understanding of
how the CPU works to optimize efficiently.

At the time of the writing, most of the information provided here is
speculative. Some of the hypotheses needs to be validated in real conditions.

* Documentation (in suggested reading order):
- http://www.realworldtech.com/haswell-cpu/
- http://www.anandtech.com/show/6355/intels-haswell-architecture
- http://www.agner.org/optimize/ (read all 5 manuals, several times!)
- http://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-optimization-manual.pdf (beware the other stale versions on Intel's website)

* Hardware summary:

Instruction fetch:      16B/cycle.
Instruction queue:      20 instructions (before instructions are decoded).
Micro-op queue:         56 fused uops (after instructions are decoded).
Micro-op cache:         1500 fused uops (about 1000 instructions).
Fused uop issue rate:   4 fused uop/cycle. Retirement is not a bottleneck.
Reorder buffer size:    192 fused uops.
Reservation stations:   60 fused uops.
In-flight loads:        72.
In-flight stores:       42.
BTB size:               Unknown. The number of loops well predicted is unknown.
L1 data cache accesses: 2x32B reads per cycle, 1x32B write per cycle, sustained.
Caches:                 L1 data 32K (512 cache lines), code 32K.
                          4 cycle latency.
                          10 line fill buffers (L1 <=> L2).
                        L2 unified 256K (4096 cache lines).
                          11 cycle latency.
                          Deliver 1 cache line per cycle to the L1 cache.
                          16 outstanding loads from L3.
                        L3 unified 8M, 21-33 cycle latency?
                        L4 (if Crystalwell): 128M, 61 cycle latency?
                        RAM: 31ns latency?
                        http://www.sisoftware.co.uk/?d=qa&f=mem_hsw

** Optimize for the biggest bottlenecks:

To optimize a function, you must identify the biggest bottlenecks and mitigate
them. The strategy depends on the bottleneck. In the sections below, we
identify the common bottlenecks and how to deal with them.

It's quite difficult to pinpoint the biggest bottleneck. Our intuition is
frequently wrong. Typically, you have to run your code in a situation most
representative of the actual workload, guess the probable bottlenecks, and try
different things to find what works best. It's an arduous process.

** Instruction fetch stalls:

The frontend can parse 16 bytes per cycle of the instruction stream. That
represents about 2 AVX2 instructions. It falls short of the 4 fused uops per
cycle that the backend can process.

The solution is the uop cache. Try to make the working set fit in the uop
cache. You have to consider the impact of all the functions of the working set
in the uop cache, not just the function you're optimizing. Use loops and
function calls to "factor out" common blocks of instructions aggressively.

It's quite common for the uop cache to be cold since the encoder has a lot of
code to run for a given block. Use loops and function calls early to start
hitting into the uop cache as soon as possible. Presumably, the instruction
fetch unit will fill out the instruction queue while the code is hitting in the
uop cache. You could also try reducing the instruction count and using smaller
instructions, e.g. make the code less load-heavy.

** Branch prediction stalls:

There are many types of branches:
- Unconditional direct jump (e.g. jmp %rip+16).
- Conditional direct jump (e.g. jne %rip+16).
- Direct call (e.g. call %rip+16).
- Indirect jump (e.g. jmp %rax).
- Indirect call (e.g. call %rax).

Every branch allocates an entry in the branch target buffer (BTB). The BTB
keeps track of where a branch goes (the target) and whether the branch is taken
or fall-through. When a branch is well predicted via the BTB, the branch
instruction and the subsequent instruction fetch is efficient. Intel keeps the
details well under lid, so we don't know how large the BTB is and what it
entails for performance. Obviously, the more branches there are, the harder it
gets for the hardware to keep track of where the branches go.

A mispredicted branch flushes the whole pipeline and costs 15-20 cycles. It's
often worthwhile to replace incertain branches with arithmetic operations and
"setcc".

*** What happens when the BTB cache is cold:

The BTB cache is often cold in practice. The prediction rules can be summarized
as follow.
  back:
  jmp back         # Predicted taken (jump to lower address).
  jmp next         # Predicted not-taken (jump to higher address).
  next:
  jmp %rax         # Predicted to fall-through (i.e. treat as not a branch).
  call %rax        # Predicted to fall-through.

For conditional jumps, make sure the static prediction is correct most of the
time. It's only a matter of swapping the 'if' and 'else' clauses.

Indirect jumps and calls are predicted to fall-through. For indirect jumps,
it's therefore preferable to put the most probable target right after the jump
instruction. The predicted target is obviously wrong for an indirect call,
making an expensive stall inevitable. Intel has hinted that the instructions
fetched are somehow parsed after the call has returned, so that it's not a
total waste.

The target of direct branches is encoded in the instruction itself. The
instruction fetch unit presumably continues to fetch and pre-decode the
instruction stream until it has identified the branch instruction and extracted
the branch target. If the CPU predicts that the branch does not fall-through,
then the instruction fetch unit start fetching at the new address. This
processing is not instantaneous and it introduces a bubble of a few cycles in
the instruction queue, which may or may not lead to a pipeline stall, depending
on the future usage of the uop cache.

The target of indirect calls is not known until the call instruction has been
executed. Hence, a full pipeline stall occurs for every indirect call. If you
need to do several indirect calls to the same target, consider using a function
stub. For example,
        # Bad: 3 flushes.
        call %r12
        call %r12
        call %r12
        # Good: Bubbles + 1 flush.
        call stub
        call stub
        call stub
        stub:
        jmp %r12

In summary, try to make sure that either the static prediction is good or that
the BTB cache is hot when you use branches.

** Reorder buffer (ROB) and reservation station (RS) stalls:

Each cycle, the ROB removes up to 4 fused uops from the micro-op queue in
execution order, and retires up to 4 finished fused uops in execution order.
The ROB allocates an entry for each fused uop in the RS, in execution order.
Presumably, the ROB buffers uops until there is room for them in the RS.

The ROB and RS can stall independently when they are full. A ROB stall occurs
when the oldest instruction in the ROB is not finished and the ROB is full. In
that case the ROB cannot take any more uop from the micro-op queue, even if the
RS is nearly empty. A RS stall occurs when the RS is full and the uops in the
RS depend on the results of previous uops that have not finished. The worst
case is when the throughput is dependent on the latency of a single
instruction, e.g. 1 instruction per 5 cycle because of dependent
multiplications.

The main causes are cache misses, long dependency chains, long instruction
latencies, low instruction throughput and execution port pressure. Compared to
Nehalem, the latency of many instructions has increased while their throughput
has decreased, and the uop issue rate has increased. RS stalls are therefore
difficult to avoid in vector code.

Interleaving several independent dependency chains help to avoid RS stalls. In
this way, the uops of concurrent chains can execute while one chain is blocked.
Splitting an algorithm in short passes over a large number of independent
elements can also help.

In general, for optimal results, you have to figure out how long your
dependency chains grow (the combined latency from all sources), which uops can
execute in parallel, which ports are available to execute those uops, and find
ways to relieve the pressure on an overtaxed execution port (typically port 5).
It's quite an ordeal.

** Store forwarding stalls:

It takes many cycles to update the L1 cache after a memory store operation.
Store-forwarding allows an instruction to read back the memory written by a
previous instruction before the store has made its way to the cache. Presumably
it takes only one cycle to load the memory from an in-flight store (assuming
the store operation has executed, of course).

The rules are complex. Read carefully Agner's manual. In general, every store
and load must be aligned on its natural data boundary. In other words, if
you're storing 32B, then the store address must be aligned on 32 bytes. There
are some exceptions for store and loads of 64 bytes or less.

Keep in mind that the compiler aligns the stack on a 16-byte boundary. If you
want to store a YMM on the stack, you first have to align the stack manually.

The data you're reading must be fully contained in the memory range of the
store. You cannot use stores to combine memory. For example, if you store two
8B values contiguously in memory and load the result as a 16B value, the load
will stall. If you want to combine memory, you must work with registers.

A load will stall for 10 to 50 cycles if the store-forwarding fails, i.e. if
the memory range you're loading from is covered by an in-flight store and the
forwarding conditions do not apply.

** Execution port stalls:

The vector execution units in Haswell are hyper-specialized. The general
break-down is as follow:
- Logic & move: port 0, 1 or 5.
- Simple arithmetic: port 1 or 5.
- Multiply & SAD: port 0.
- Constant shift: port 0.
- Variable shift: port 0 & 5.
- Shuffle: port 5.
- VPBLENDD: port 0, 1, or 5.

There are plenty of exceptions. Some instructions use different ports if they
operate with memory operands instead of registers. Check your instruction table
carefully.

*** Cross-lane stalls:

Any instruction that mixes the data from the high and low part of a YMM has a
minimum latency of 3 cycles, even for memory operands. There is a virtual fence
between the two halves of a YMM register, and it's hard to cross. It is not
possible to reorder a full YMM arbitrarily (at the byte level) with a single
instruction. Thus, work vertically when possible. There is no good solution to
this problem.

*** Port 5 pressure.

Port 5 is often the bottleneck. Only port 5 can handle shuffles, and it's also
used for other instructions. If you need to shuffle data heavily, there are
some ugly ways to shuffle data without using port 5.
- Use constant shifts to align data horizontally.
- Use logical operations or VPBLENDD to combine data vertically.
- Use VINSERTI128 with memory operands to swap the high and low parts of a YMM.

The Intel optimization manual has an example of how to reduce port 5 pressure
when transposing a matrix.

*** Using load-heavy code and port 6:

With a sustained throughput of 2 loads and 1 store per cycle, memory/register
transfers are cheap. Notice that many instructions use the same number of fused
uops regardless of whether they use register or memory operands. Memory loads
and stores are handled by the ports 2/3/4 and they don't interfere with the
ports 0/1/5 used for the execution of most instructions. If you can live with
the latency, you can use load-heavy code to reduce the instruction count or
work around a busy execution port. Keep in mind that fused uops count as 1 uop
in the uop cache but the instructions themselves take more bytes to encode when
they use memory operands.

If you find that an execution port is a major bottleneck and that you're not
quite executing 4 fused uops per cycle, consider throwing in some load/store
operations or do GPR "busy-work" with ports 0/1/5/6 in any way that help making
use of the available execution bandwidth that is otherwise going to waste.

