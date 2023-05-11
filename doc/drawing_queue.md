
# GPU drawing queue

`libpsxgpu` manages access to the GPU by implementing a software driven queue.
This queue, separate from the GPU's internal command FIFO, allows for high-level
management of GPU operations such as display list sending, VRAM image uploads
and framebuffer readback, in a similar way to the drawing queue system
implemented behind the scenes by the official SDK.

The queue is managed internally by the library and can hold up to 16 drawing
operations ("DrawOps"). Each DrawOp is represented by a pointer to a function,
alongside any arguments to be passed to it. Whenever the GPU is idle,
`libpsxgpu` fetches a DrawOp from the queue and calls its respective function,
which should then proceed to actually send commands to the GPU or set up and
start a DMA transfer. `DrawSync()` can be called to wait for the queue to become
empty or get its current length, while `DrawSyncCallback()` may be used to
register a callback that will be invoked once the GPU is idle and no more
DrawOps are pending.

Completion of each DrawOp (and transition of the GPU from busy to idle state) is
signalled through one of two means:

- the DMA channel 2 IRQ, fired automatically by the DMA unit when a data
  transfer such as a VRAM upload or a display list has finished executing;
- the GPU IRQ, triggered manually using the `GP0(0x1f)` command or the `DR_IRQ`
  primitive.

Note that the end of a DMA transfer does not necessarily imply that the GPU has
finished executing all commands; the last command issued may not yet be done,
hence the ability to use the GPU IRQ instead is provided as a more reliable way
to detect the completion of certain commands.

## Built-in DrawOps

The library includes a number of built-in DrawOps for the most common use cases.
The following APIs are wrappers around DrawOps:

- `DrawBuffer()` and `DrawBufferIRQ()` queue a new DrawOp to start a DMA
   transfer in chunked mode (sending one word at a time) with the specified
   starting address and number of words. `DrawBuffer2()` and `DrawBufferIRQ2()`
   are the underlying DrawOp functions respectively.
- `DrawOTag()` and `DrawOTagIRQ()` queue a new DrawOp to start a DMA transfer in
   linked-list mode with the specified starting address, with `DrawOTag2()` and
   `DrawOTagIRQ2()` being the respective DrawOp functions.
- `PutDrawEnv()`, `PutDrawEnvFast()`, `DrawOTagEnv()` and `DrawOTagEnvIRQ()`
   insert drawing environment setup commands as the first (or only) item in a
   display list, then proceed to pass it to `DrawOTag()`. The setup packet
   linked into the display list is stored as part of the `DRAWENV` structure.
- `LoadImage()` and `StoreImage()` copy the provided coordinates into a
  temporary buffer, then proceed to enqueue a DrawOp to actually start the VRAM
  transfer. The synchronous variants of these APIs are `LoadImage2()` and
  `StoreImage2()` respectively.
- `MoveImage()` saves the provided coordinates into a temporary buffer, then
   enqueues a DrawOp that will issue a `GP0(0x80)` VRAM blitting command. As
   this command is handled entirely by the GPU with no DMA transfers involved,
   the GPU IRQ is used to detect its completion.

## Custom DrawOps

Unlike the official SDK, `libpsxgpu` exposes the drawing queue by providing a
way to enqueue arbitrary custom DrawOps. This can be useful for profiling
purposes or to work around specific GPU bugs (see the use cases section).

Custom DrawOps can be pushed into the queue by calling `EnqueueDrawOp()` and
passing a pointer to the callback function in charge of issuing the DrawOp's
commands to the GPU, as well as up to 3 arguments to be passed through to it.
The function must:

- call `SetDrawOpType()` to let the library know which type of IRQ it shall wait
  for before moving onto the next DrawOp (either `DRAWOP_TYPE_DMA` or
  `DRAWOP_TYPE_GPU_IRQ`);
- wait until the GPU is ready to accept commands by polling the status bits in
  `GPU_STAT` and make sure DMA channel 2 is also idle before proceeding;
- issue any commands to the GPU's GP0 register and/or set up a DMA transfer,
  terminating them with a `GP0(0x1f)` IRQ command if appropriate.

Note that DrawOps are called from within the exception handler's context and
must thus not block for significant periods of time, manipulate COP0 registers
or wait for any IRQs to occur. They are also restricted from manipulating the
drawing queue by e.g. calling `EnqueueDrawOp()`, `DrawOTag()` or any other
function that enqueues a DrawOp.

## Use cases

### Scissoring commands

The GPU provides commands to set the origin of all X/Y coordinates passed to it
as well as a scissoring region, all pixels outside of which are automatically
masked out during drawing. These commands are issued to the GP0 register and can
be inserted in a display list through the `DR_OFFSET` and `DR_AREA` primitives,
however they will *not* go through the GPU's command FIFO like most other
primitives. They will instead take effect immediately, resulting in graphical
glitches if the GPU is already busy processing a drawing command (i.e. if they
are not the very first commands in a display list).

The software-driven drawing queue provides a way around this. By splitting up a
frame's display list into multiple chunks, one for each scissoring command
issued, it is possible to always place scissoring commands at the beginning of a
chunk. Each chunk can be terminated with a `DR_IRQ` primitive and queued for
drawing using `DrawOTagIRQ()` to ensure the GPU goes idle before the next chunk
is sent, preventing scissoring commands from being received by the GPU while
busy.

-----------------------------------------
_Last updated on 2023-05-11 by spicyjpeg_
