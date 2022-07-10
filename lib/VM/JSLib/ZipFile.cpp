#include "JSLibInternal.h"

#include "hermes/VM/JSArrayBuffer.h"
#include "hermes/VM/JSZipFile.h"
#include "zip/src/zip.h"

namespace hermes {
namespace vm {

// new ZipFile(path: string, level: number, mode: string)
CallResult<HermesValue>
zipFileConstructor(void *, Runtime *runtime, NativeArgs args) {
  if (!args.isConstructorCall()) {
    return runtime->raiseTypeError(
        "ZipFile() called in function context instead of constructor");
  }

  auto self = args.vmcastThis<JSZipFile>();

  auto pathHandle = args.dyncastArg<StringPrimitive>(0);
  if (!pathHandle) {
    return runtime->raiseTypeError("Path has to be a string");
  }
  auto path = toString(runtime, pathHandle);

  auto level = args.getArg(1);
  if (!level.isNumber()) {
    return runtime->raiseTypeError("Level has to be a number");
  }

  auto modeHandle = args.dyncastArg<StringPrimitive>(2);
  if (!modeHandle) {
    return runtime->raiseTypeError("Mode has to be a string");
  }
  auto mode = toString(runtime, modeHandle);
  if (mode.length() != 1) {
    return runtime->raiseTypeError("Mode has to be a single character");
  }

  ::hermes::hermesLog(
      "AliuHermes",
      "new ZipFile(%s, %d, %s)",
      path.c_str(),
      level.getNumberAs<int>(),
      mode.c_str());

  if (auto zip =
          zip_open(path.c_str(), level.getNumberAs<int>(), mode.c_str()[0])) {
    self->set(zip);
  } else {
    return runtime->raiseError("Failed to open the zip");
  }

  return self.getHermesValue();
}

// ZipFile.openEntry(name: string)
CallResult<HermesValue>
zipFileOpenEntry(void *, Runtime *runtime, NativeArgs args) {
  auto self = args.vmcastThis<JSZipFile>();
  if (!self) {
    return runtime->raiseTypeError(
        "ZipFile.prototype.openEntry() called on non-ZipFile object");
  }

  auto zip = self->get();
  if (!zip) {
    return runtime->raiseError("This zip is already closed");
  }

  auto nameHandle = args.dyncastArg<StringPrimitive>(0);
  if (!nameHandle) {
    return runtime->raiseTypeError("Name has to be a string");
  }
  auto name = toString(runtime, nameHandle);

  auto status = zip_entry_open(zip, name.c_str());
  if (status < 0) {
    return runtime->raiseError(zip_strerror(status));
  }

  return HermesValue::encodeUndefinedValue();
}

// ZipFile.readEntry(encoding: "text" | "binary"): string | ArrayBuffer
CallResult<HermesValue>
zipFileReadEntry(void *, Runtime *runtime, NativeArgs args) {
  auto self = args.vmcastThis<JSZipFile>();
  if (!self) {
    return runtime->raiseTypeError(
        "ZipFile.prototype.readEntry() called on non-ZipFile object");
  }

  auto zip = self->get();
  if (!zip) {
    return runtime->raiseError("This zip is already closed");
  }

  auto encodingHandle = args.dyncastArg<StringPrimitive>(0);
  if (!encodingHandle) {
    return runtime->raiseTypeError("Encoding has to be a string");
  }
  auto encoding = toString(runtime, encodingHandle);

  size_t size;

  if (encoding == "text") {
    void *data = nullptr;

    auto status = zip_entry_read(zip, &data, &size);
    if (status < 0) {
      return runtime->raiseError(zip_strerror(status));
    }

    return runtime
        ->makeHandle<StringPrimitive>(
            *StringPrimitive::create(runtime, ASCIIRef((char *)data, size)))
        .getHermesValue();
  }

  if (encoding == "binary") {
    size = zip_entry_size(zip);

    auto buffer = runtime->makeHandle(JSArrayBuffer::create(
        runtime, Handle<JSObject>::vmcast(&runtime->arrayBufferPrototype)));
    if (LLVM_UNLIKELY(
            buffer->createDataBlock(runtime, size, false) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    auto status = zip_entry_noallocread(zip, buffer->getDataBlock(), size);
    if (status < 0) {
      return runtime->raiseError(zip_strerror(status));
    }

    return buffer.getHermesValue();
  }

  return runtime->raiseTypeError("Encoding has to be \"text\" or \"binary\"");
}

// ZipFile.closeEntry()
CallResult<HermesValue>
zipFileCloseEntry(void *, Runtime *runtime, NativeArgs args) {
  auto self = args.vmcastThis<JSZipFile>();
  if (!self) {
    return runtime->raiseTypeError(
        "ZipFile.prototype.closeEntry() called on non-ZipFile object");
  }

  auto zip = self->get();
  if (!zip) {
    return runtime->raiseError("This zip is already closed");
  }

  auto status = zip_entry_close(zip);
  if (status < 0) {
    return runtime->raiseError(zip_strerror(status));
  }

  return HermesValue::encodeUndefinedValue();
}

// ZipFile.close()
CallResult<HermesValue>
zipFileClose(void *, Runtime *runtime, NativeArgs args) {
  auto self = args.vmcastThis<JSZipFile>();
  if (!self) {
    return runtime->raiseTypeError(
        "ZipFile.prototype.close() called on non-ZipFile object");
  }

  zip_close(self->get());
  self->set(nullptr);

  return HermesValue::encodeUndefinedValue();
}

Handle<JSObject> createZipFileConstructor(Runtime *runtime) {
  auto zipFilePrototype = Handle<JSObject>::vmcast(&runtime->zipFilePrototype);
  auto cons = defineSystemConstructor<JSZipFile>(
      runtime,
      Predefined::getSymbolID(Predefined::ZipFile),
      zipFileConstructor,
      zipFilePrototype,
      3,
      CellKind::ZipFileKind);

  defineMethod(
      runtime,
      zipFilePrototype,
      Predefined::getSymbolID(Predefined::openEntry),
      nullptr,
      zipFileOpenEntry,
      1);

  defineMethod(
      runtime,
      zipFilePrototype,
      Predefined::getSymbolID(Predefined::readEntry),
      nullptr,
      zipFileReadEntry,
      1);

  defineMethod(
      runtime,
      zipFilePrototype,
      Predefined::getSymbolID(Predefined::closeEntry),
      nullptr,
      zipFileCloseEntry,
      0);

  defineMethod(
      runtime,
      zipFilePrototype,
      Predefined::getSymbolID(Predefined::close),
      nullptr,
      zipFileClose,
      0);

  return cons;
}

} // namespace vm
} // namespace hermes
