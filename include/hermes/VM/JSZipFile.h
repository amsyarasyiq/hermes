#ifndef HERMES_VM_JSZIPFILE_H
#define HERMES_VM_JSZIPFILE_H

#include "hermes/VM/JSObject.h"
#include "zip/src/zip.h"

namespace hermes {
namespace vm {

class JSZipFile final : public JSObject {
  using Super = JSObject;
  friend void ZipFileBuildMeta(const GCCell *, Metadata::Builder &);

 public:
  static const ObjectVTable vt;

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::ZipFileKind;
  }

  static PseudoHandle<JSZipFile>
  create(Runtime &runtime, zip_t *zip, Handle<JSObject> prototype);

  static PseudoHandle<JSZipFile> create(
      Runtime &runtime,
      Handle<JSObject> prototype) {
    return create(runtime, nullptr, prototype);
  }

  zip_t *get() const {
    return zip_;
  }

  void set(zip_t *zip) {
    zip_ = zip;
  }

  JSZipFile(
      Runtime &runtime,
      zip_t *zip,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz)
      : JSObject(runtime, *parent, *clazz), zip_{zip} {}

 private:
  zip_t *zip_;
};

} // namespace vm
} // namespace hermes

#endif
