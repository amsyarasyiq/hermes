#include "hermes/VM/JSZipFile.h"

#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/Runtime-inline.h"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// class JSZipFile

const ObjectVTable JSZipFile::vt{
    VTable(CellKind::ZipFileKind, cellSize<JSZipFile>()),
    JSZipFile::_getOwnIndexedRangeImpl,
    JSZipFile::_haveOwnIndexedImpl,
    JSZipFile::_getOwnIndexedPropertyFlagsImpl,
    JSZipFile::_getOwnIndexedImpl,
    JSZipFile::_setOwnIndexedImpl,
    JSZipFile::_deleteOwnIndexedImpl,
    JSZipFile::_checkAllOwnIndexedImpl,
};

void ZipFileBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSZipFile>());
  JSObjectBuildMeta(cell, mb);
  mb.setVTable(&JSZipFile::vt);
}

PseudoHandle<JSZipFile>
JSZipFile::create(Runtime &runtime, zip_t *zip, Handle<JSObject> parentHandle) {
  auto *cell = runtime.makeAFixed<JSZipFile>(
      runtime,
      zip,
      parentHandle,
      runtime.getHiddenClassForPrototype(
          *parentHandle, numOverlapSlots<JSZipFile>()));
  return JSObjectInit::initToPseudoHandle(runtime, cell);
}

} // namespace vm
} // namespace hermes
