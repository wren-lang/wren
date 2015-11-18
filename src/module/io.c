#include <stdlib.h>
#include <string.h>

#include "uv.h"

#include "scheduler.h"
#include "vm.h"
#include "wren.h"

#include <stdio.h>

#ifdef _WIN32
  #include <fcntl.h>
#endif

static const int stdinDescriptor = 0;

// Handle to Stdin.onData_(). Called when libuv provides data on stdin.
static WrenValue* stdinOnData = NULL;

// The stream used to read from stdin. Initialized on the first read.
static uv_stream_t* stdinStream = NULL;

// Frees all resources related to stdin.
static void shutdownStdin()
{
  if (stdinStream != NULL)
  {
    uv_close((uv_handle_t*)stdinStream, NULL);
    free(stdinStream);
    stdinStream = NULL;
  }
  
  if (stdinOnData != NULL)
  {
    wrenReleaseValue(getVM(), stdinOnData);
    stdinOnData = NULL;
  }
}

void ioShutdown()
{
  shutdownStdin();
}

void fileAllocate(WrenVM* vm)
{
  // Store the file descriptor in the foreign data, so that we can get to it
  // in the finalizer.
  int* fd = (int*)wrenAllocateForeign(vm, sizeof(int));
  *fd = (int)wrenGetArgumentDouble(vm, 1);
}

void fileFinalize(WrenVM* vm)
{
  int fd = *(int*)wrenGetArgumentForeign(vm, 0);
  
  // Already closed.
  if (fd == -1) return;
  
  uv_fs_t request;
  uv_fs_close(getLoop(), &request, fd, NULL);
  uv_fs_req_cleanup(&request);
}

// If [request] failed with an error, sends the runtime error to the VM and
// frees the request.
//
// Returns true if an error was reported.
static bool handleRequestError(uv_fs_t* request)
{
  if (request->result >= 0) return false;
  
  WrenValue* fiber = (WrenValue*)request->data;
  schedulerResumeError(fiber, uv_strerror((int)request->result));
  uv_fs_req_cleanup(request);
  free(request);
  return true;
}

// Allocates a new request that resumes [fiber] when it completes.
uv_fs_t* createRequest(WrenValue* fiber)
{
  uv_fs_t* request = (uv_fs_t*)malloc(sizeof(uv_fs_t));
  request->data = fiber;
  return request;
}

// Releases resources used by [request].
//
// Returns the fiber that should be resumed after [request] completes.
WrenValue* freeRequest(uv_fs_t* request)
{
  WrenValue* fiber = (WrenValue*)request->data;
  
  uv_fs_req_cleanup(request);
  free(request);

  return fiber;
}

static void openCallback(uv_fs_t* request)
{
  if (handleRequestError(request)) return;
  
  double fd = (double)request->result;
  WrenValue* fiber = freeRequest(request);
  
  schedulerResumeDouble(fiber, fd);
}

void fileOpen(WrenVM* vm)
{
  const char* path = wrenGetArgumentString(vm, 1);
  uv_fs_t* request = createRequest(wrenGetArgumentValue(vm, 2));
  
  // TODO: Allow controlling flags and modes.
  uv_fs_open(getLoop(), request, path, O_RDONLY, 0, openCallback);
}

// Called by libuv when the stat call for size completes.
static void sizeCallback(uv_fs_t* request)
{
  if (handleRequestError(request)) return;
  
  double size = (double)request->statbuf.st_size;
  WrenValue* fiber = freeRequest(request);
  
  schedulerResumeDouble(fiber, size);
}

void fileSizePath(WrenVM* vm)
{
  const char* path = wrenGetArgumentString(vm, 1);
  uv_fs_t* request = createRequest(wrenGetArgumentValue(vm, 2));
  uv_fs_stat(getLoop(), request, path, sizeCallback);
}

static void closeCallback(uv_fs_t* request)
{
  if (handleRequestError(request)) return;
  
  WrenValue* fiber = freeRequest(request);
  schedulerResume(fiber);
}

void fileClose(WrenVM* vm)
{
  int* foreign = (int*)wrenGetArgumentForeign(vm, 0);
  int fd = *foreign;
  
  // If it's already closed, we're done.
  if (fd == -1)
  {
    wrenReturnBool(vm, true);
    return;
  }
  
  // Mark it closed immediately.
  *foreign = -1;
  
  uv_fs_t* request = createRequest(wrenGetArgumentValue(vm, 1));
  uv_fs_close(getLoop(), request, fd, closeCallback);
  wrenReturnBool(vm, false);
}

void fileDescriptor(WrenVM* vm)
{
  int* foreign = (int*)wrenGetArgumentForeign(vm, 0);
  int fd = *foreign;
  wrenReturnDouble(vm, fd);
}

static void fileReadBytesCallback(uv_fs_t* request)
{
  if (handleRequestError(request)) return;

  uv_buf_t buffer = request->fs.info.bufs[0];
  WrenValue* fiber = freeRequest(request);

  // TODO: Having to copy the bytes here is a drag. It would be good if Wren's
  // embedding API supported a way to *give* it bytes that were previously
  // allocated using Wren's own allocator.
  schedulerResumeBytes(fiber, buffer.base, buffer.len);
  
  // TODO: Likewise, freeing this after we resume is lame.
  free(buffer.base);
}

void fileReadBytes(WrenVM* vm)
{
  uv_fs_t* request = createRequest(wrenGetArgumentValue(vm, 2));
  
  int fd = *(int*)wrenGetArgumentForeign(vm, 0);
  // TODO: Assert fd != -1.

  uv_buf_t buffer;
  buffer.len = (size_t)wrenGetArgumentDouble(vm, 1);
  buffer.base = (char*)malloc(buffer.len);
  
  // TODO: Allow passing in offset.
  uv_fs_read(getLoop(), request, fd, &buffer, 1, 0, fileReadBytesCallback);
}

void fileSize(WrenVM* vm)
{
  uv_fs_t* request = createRequest(wrenGetArgumentValue(vm, 1));

  int fd = *(int*)wrenGetArgumentForeign(vm, 0);
  // TODO: Assert fd != -1.
  
  uv_fs_fstat(getLoop(), request, fd, sizeCallback);
}

static void allocCallback(uv_handle_t* handle, size_t suggestedSize,
                          uv_buf_t* buf)
{
  // TODO: Handle allocation failure.
  buf->base = (char*)malloc(suggestedSize);
  buf->len = suggestedSize;
}

static void stdinReadCallback(uv_stream_t* stream, ssize_t numRead,
                               const uv_buf_t* buffer)
{
  // If stdin was closed, send null to let io.wren know.
  if (numRead == UV_EOF)
  {
    wrenCall(getVM(), stdinOnData, NULL, "v", NULL);
    shutdownStdin();
    return;
  }
  
  // TODO: Handle other errors.
  
  if (stdinOnData == NULL)
  {
    stdinOnData = wrenGetMethod(getVM(), "io", "Stdin", "onData_(_)");
  }

  // TODO: Having to copy the bytes here is a drag. It would be good if Wren's
  // embedding API supported a way to *give* it bytes that were previously
  // allocated using Wren's own allocator.
  wrenCall(getVM(), stdinOnData, NULL, "a", buffer->base, numRead);
  
  // TODO: Likewise, freeing this after we resume is lame.
  free(buffer->base);
}

void stdinReadStart(WrenVM* vm)
{
  if (stdinStream == NULL)
  {
    if (uv_guess_handle(stdinDescriptor) == UV_TTY)
    {
      // stdin is connected to a terminal.
      uv_tty_t* handle = (uv_tty_t*)malloc(sizeof(uv_tty_t));
      uv_tty_init(getLoop(), handle, stdinDescriptor, true);
      stdinStream = (uv_stream_t*)handle;
    }
    else
    {
      // stdin is a pipe or a file.
      uv_pipe_t* handle = (uv_pipe_t*)malloc(sizeof(uv_pipe_t));
      uv_pipe_init(getLoop(), handle, false);
      uv_pipe_open(handle, stdinDescriptor);
      stdinStream = (uv_stream_t*)handle;
    }
  }
  
  uv_read_start(stdinStream, allocCallback, stdinReadCallback);
  // TODO: Check return.
}

void stdinReadStop(WrenVM* vm)
{
  uv_read_stop(stdinStream);
}
