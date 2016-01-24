#include <stdlib.h>
#include <string.h>

#include "uv.h"

#include "scheduler.h"
#include "vm.h"
#include "wren.h"

#include <stdio.h>
#include <fcntl.h>

typedef struct sFileRequestData
{
  WrenValue* fiber;
  uv_buf_t buffer;
} FileRequestData;

static const int stdinDescriptor = 0;

// Handle to the Stdin class object.
static WrenValue* stdinClass = NULL;

// Handle to an onData_() method call. Called when libuv provides data on stdin.
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
  
  if (stdinClass != NULL)
  {
    wrenReleaseValue(getVM(), stdinClass);
    stdinClass = NULL;
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

// If [request] failed with an error, sends the runtime error to the VM and
// frees the request.
//
// Returns true if an error was reported.
static bool handleRequestError(uv_fs_t* request)
{
  if (request->result >= 0) return false;

  FileRequestData* data = (FileRequestData*)request->data;
  WrenValue* fiber = (WrenValue*)data->fiber;
  
  schedulerResumeError(fiber, uv_strerror((int)request->result));
  
  free(data);
  uv_fs_req_cleanup(request);
  free(request);
  return true;
}

// Allocates a new request that resumes [fiber] when it completes.
uv_fs_t* createRequest(WrenValue* fiber)
{
  uv_fs_t* request = (uv_fs_t*)malloc(sizeof(uv_fs_t));
  
  FileRequestData* data = (FileRequestData*)malloc(sizeof(FileRequestData));
  data->fiber = fiber;
  
  request->data = data;
  return request;
}

// Releases resources used by [request].
//
// Returns the fiber that should be resumed after [request] completes.
WrenValue* freeRequest(uv_fs_t* request)
{
  FileRequestData* data = (FileRequestData*)request->data;
  WrenValue* fiber = data->fiber;
  
  free(data);
  uv_fs_req_cleanup(request);
  free(request);
  
  return fiber;
}

static void directoryListCallback(uv_fs_t* request)
{
  if (handleRequestError(request)) return;

  uv_dirent_t entry;

  WrenVM* vm = getVM();
  wrenEnsureSlots(vm, 3);
  wrenSetSlotNewList(vm, 2);
  
  while (uv_fs_scandir_next(request, &entry) != UV_EOF)
  {
    wrenSetSlotString(vm, 1, entry.name);
    wrenInsertInList(vm, 2, -1, 1);
  }
  
  schedulerResume(freeRequest(request), true);
  schedulerFinishResume();
}

void directoryList(WrenVM* vm)
{
  const char* path = wrenGetSlotString(vm, 1);
  
  uv_fs_t* request = createRequest(wrenGetSlotValue(vm, 2));
  
  // TODO: Check return.
  uv_fs_scandir(getLoop(), request, path, 0, directoryListCallback);
}

void fileAllocate(WrenVM* vm)
{
  // Store the file descriptor in the foreign data, so that we can get to it
  // in the finalizer.
  int* fd = (int*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(int));
  *fd = (int)wrenGetSlotDouble(vm, 1);
}

void fileFinalize(void* data)
{
  int fd = *(int*)data;
  
  // Already closed.
  if (fd == -1) return;
  
  uv_fs_t request;
  uv_fs_close(getLoop(), &request, fd, NULL);
  uv_fs_req_cleanup(&request);
}

static void fileOpenCallback(uv_fs_t* request)
{
  if (handleRequestError(request)) return;
  
  double fd = (double)request->result;
  schedulerResume(freeRequest(request), true);
  wrenSetSlotDouble(getVM(), 2, fd);
  schedulerFinishResume();
}

void fileOpen(WrenVM* vm)
{
  const char* path = wrenGetSlotString(vm, 1);
  uv_fs_t* request = createRequest(wrenGetSlotValue(vm, 2));

  // TODO: Allow controlling flags and modes.
  uv_fs_open(getLoop(), request, path, O_RDONLY, 0, fileOpenCallback);
}

// Called by libuv when the stat call for size completes.
static void fileSizeCallback(uv_fs_t* request)
{
  if (handleRequestError(request)) return;

  double size = (double)request->statbuf.st_size;
  schedulerResume(freeRequest(request), true);
  wrenSetSlotDouble(getVM(), 2, size);
  schedulerFinishResume();
}

void fileSizePath(WrenVM* vm)
{
  const char* path = wrenGetSlotString(vm, 1);
  uv_fs_t* request = createRequest(wrenGetSlotValue(vm, 2));
  uv_fs_stat(getLoop(), request, path, fileSizeCallback);
}

// Called by libuv when the stat call completes.
static void fileStatPathCallback(uv_fs_t* request)
{
  if (handleRequestError(request)) return;
  
  WrenVM* vm = getVM();
  wrenEnsureSlots(vm, 4);
  wrenSetSlotNewList(vm, 2);
  
  wrenSetSlotDouble(vm, 3, (double)request->statbuf.st_dev);
  wrenInsertInList(vm, 2, -1, 3);
  
  wrenSetSlotDouble(vm, 3, (double)request->statbuf.st_ino);
  wrenInsertInList(vm, 2, -1, 3);
  
  wrenSetSlotDouble(vm, 3, (double)request->statbuf.st_mode);
  wrenInsertInList(vm, 2, -1, 3);
  
  wrenSetSlotDouble(vm, 3, (double)request->statbuf.st_nlink);
  wrenInsertInList(vm, 2, -1, 3);
  
  wrenSetSlotDouble(vm, 3, (double)request->statbuf.st_uid);
  wrenInsertInList(vm, 2, -1, 3);
  
  wrenSetSlotDouble(vm, 3, (double)request->statbuf.st_gid);
  wrenInsertInList(vm, 2, -1, 3);

  wrenSetSlotDouble(vm, 3, (double)request->statbuf.st_rdev);
  wrenInsertInList(vm, 2, -1, 3);
  
  wrenSetSlotDouble(vm, 3, (double)request->statbuf.st_size);
  wrenInsertInList(vm, 2, -1, 3);
  
  wrenSetSlotDouble(vm, 3, (double)request->statbuf.st_blksize);
  wrenInsertInList(vm, 2, -1, 3);
  
  wrenSetSlotDouble(vm, 3, (double)request->statbuf.st_blocks);
  wrenInsertInList(vm, 2, -1, 3);

  // TODO: Include access, modification, and change times once we figure out
  // how we want to represent it.
  //  time_t    st_atime;   /* time of last access */
  //  time_t    st_mtime;   /* time of last modification */
  //  time_t    st_ctime;   /* time of last status change */

  schedulerResume(freeRequest(request), true);
  schedulerFinishResume();
}

void fileStatPath(WrenVM* vm)
{
  const char* path = wrenGetSlotString(vm, 1);
  uv_fs_t* request = createRequest(wrenGetSlotValue(vm, 2));
  uv_fs_stat(getLoop(), request, path, fileStatPathCallback);
}

static void fileCloseCallback(uv_fs_t* request)
{
  if (handleRequestError(request)) return;

  schedulerResume(freeRequest(request), false);
}

void fileClose(WrenVM* vm)
{
  int* foreign = (int*)wrenGetSlotForeign(vm, 0);
  int fd = *foreign;

  // If it's already closed, we're done.
  if (fd == -1)
  {
    wrenSetSlotBool(vm, 0, true);
    return;
  }

  // Mark it closed immediately.
  *foreign = -1;

  uv_fs_t* request = createRequest(wrenGetSlotValue(vm, 1));
  uv_fs_close(getLoop(), request, fd, fileCloseCallback);
  wrenSetSlotBool(vm, 0, false);
}

void fileDescriptor(WrenVM* vm)
{
  int* foreign = (int*)wrenGetSlotForeign(vm, 0);
  int fd = *foreign;
  wrenSetSlotDouble(vm, 0, fd);
}

static void fileReadBytesCallback(uv_fs_t* request)
{
  if (handleRequestError(request)) return;

  FileRequestData* data = (FileRequestData*)request->data;
  uv_buf_t buffer = data->buffer;
  size_t count = request->result;

  // TODO: Having to copy the bytes here is a drag. It would be good if Wren's
  // embedding API supported a way to *give* it bytes that were previously
  // allocated using Wren's own allocator.
  schedulerResume(freeRequest(request), true);
  wrenSetSlotBytes(getVM(), 2, buffer.base, count);
  schedulerFinishResume();

  // TODO: Likewise, freeing this after we resume is lame.
  free(buffer.base);
}

void fileReadBytes(WrenVM* vm)
{
  uv_fs_t* request = createRequest(wrenGetSlotValue(vm, 3));

  int fd = *(int*)wrenGetSlotForeign(vm, 0);
  // TODO: Assert fd != -1.

  FileRequestData* data = (FileRequestData*)request->data;
  size_t length = (size_t)wrenGetSlotDouble(vm, 1);
  size_t offset = (size_t)wrenGetSlotDouble(vm, 2);

  data->buffer.len = length;
  data->buffer.base = (char*)malloc(length);

  uv_fs_read(getLoop(), request, fd, &data->buffer, 1, offset,
             fileReadBytesCallback);
}

void fileSize(WrenVM* vm)
{
  uv_fs_t* request = createRequest(wrenGetSlotValue(vm, 1));

  int fd = *(int*)wrenGetSlotForeign(vm, 0);
  // TODO: Assert fd != -1.

  uv_fs_fstat(getLoop(), request, fd, fileSizeCallback);
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
  WrenVM* vm = getVM();
  
  if (stdinClass == NULL)
  {
    wrenEnsureSlots(vm, 1);
    wrenGetVariable(vm, "io", "Stdin", 0);
    stdinClass = wrenGetSlotValue(vm, 0);
  }
  
  if (stdinOnData == NULL)
  {
    stdinOnData = wrenMakeCallHandle(vm, "onData_(_)");
  }
  
  // If stdin was closed, send null to let io.wren know.
  if (numRead == UV_EOF)
  {
    wrenEnsureSlots(vm, 2);
    wrenSetSlotValue(vm, 0, stdinClass);
    wrenSetSlotNull(vm, 1);
    wrenCall(vm, stdinOnData);
    
    shutdownStdin();
    return;
  }

  // TODO: Handle other errors.

  // TODO: Having to copy the bytes here is a drag. It would be good if Wren's
  // embedding API supported a way to *give* it bytes that were previously
  // allocated using Wren's own allocator.
  wrenEnsureSlots(vm, 2);
  wrenSetSlotValue(vm, 0, stdinClass);
  wrenSetSlotBytes(vm, 1, buffer->base, numRead);
  wrenCall(vm, stdinOnData);

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
