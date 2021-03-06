#include "py_binary_stream.hpp"

#include <boost/python.hpp>
#include <boost/python/detail/wrap_python.hpp>

#include <medusa/binary_stream.hpp>

namespace bp = boost::python;

MEDUSA_NAMESPACE_USE

namespace pydusa
{
  void (FileBinaryStream::*pFileBinaryStream_Open)(boost::filesystem::path const&) = &FileBinaryStream::Open;

  static bp::str FileBinaryStream_Read(FileBinaryStream *pBinStrm, unsigned int Offset, size_t Size)
  {
    char *pBuffer = new char[Size];
    pBinStrm->Read(Offset, pBuffer, Size);
    return bp::str(pBuffer, Size);
  }

  static void MemoryBinaryStream_Open(MemoryBinaryStream *pBinStrm, bp::str s)
  {
    char *pBuf;
    Py_ssize_t Size;

#if PY_VERSION_HEX < 0x3000000
    PyString_AsStringAndSize(s.ptr(), &pBuf, &Size);
#else
    PyBytes_AsStringAndSize(s.ptr(), &pBuf, &Size);
#endif
    pBinStrm->Open(pBuf, static_cast<u32>(Size));
  }

  static bp::str MemoryBinaryStream_Read(MemoryBinaryStream *pBinStrm, unsigned int Offset, size_t Size)
  {
    char *pBuffer = new char[Size];
    pBinStrm->Read(Offset, pBuffer, Size);
    return bp::str(pBuffer, Size);
  }
}

void PydusaBinaryStream(void)
{
  /* Binary stream */

  bp::class_<FileBinaryStream, boost::noncopyable>("FileBinaryStream", bp::init<>())
    .def("open", pydusa::pFileBinaryStream_Open)
    .def("read",     &pydusa::FileBinaryStream_Read)
    .def("close",    &FileBinaryStream::Close)
    .def("__len__",  &FileBinaryStream::GetSize)
    ;

  bp::class_<MemoryBinaryStream, boost::noncopyable>("MemoryBinaryStream", bp::init<>())
    .def("open",    &pydusa::MemoryBinaryStream_Open)
    .def("read",    &pydusa::MemoryBinaryStream_Read)
    .def("close",   &MemoryBinaryStream::Close      )
    .def("__len__", &MemoryBinaryStream::GetSize    )
    ;
}
