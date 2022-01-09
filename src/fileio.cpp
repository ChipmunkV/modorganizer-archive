/*
Mod Organizer archive handling

Copyright (C) 2020 MO2 Team. All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "fileio.h"

inline bool BOOLToBool(BOOL v) { return (v != FALSE); }

namespace IO {

  // FileBase

  bool FileBase::Close() noexcept {
#ifdef _WIN32
    if (m_Handle == INVALID_HANDLE_VALUE)
      return true;
    if (!::CloseHandle(m_Handle))
      return false;
    m_Handle = INVALID_HANDLE_VALUE;
    return true;
#else
    if (!m_Stream.is_open())
      return true;
    if (m_Stream.close(), m_Stream.fail())
      return false;
    m_Stream = std::fstream();
    return true;
#endif
  }

  bool FileBase::GetPosition(UInt64& position) noexcept {
#ifdef _WIN32
    return Seek(0, FILE_CURRENT, position);
#else
    if (!m_Stream.is_open() || !(m_OpenMode & (std::ios::in | std::ios::out)))
      return false;
    const auto pos = m_OpenMode & std::ios::in ? m_Stream.tellg() : m_Stream.tellp();
    if (!m_Stream.good())
      return false;
    position = pos;
    return true;
#endif
  }

  bool FileBase::GetLength(UInt64& length) const noexcept {
#ifdef _WIN32
    DWORD sizeHigh;
    DWORD sizeLow = ::GetFileSize(m_Handle, &sizeHigh);
    if (sizeLow == 0xFFFFFFFF)
      if (::GetLastError() != NO_ERROR)
        return false;
    length = (((UInt64)sizeHigh) << 32) + sizeLow;
    return true;
#else
    if (!m_Stream.is_open() || !(m_OpenMode & (std::ios::in | std::ios::out)))
      return false;
    std::fstream stream(m_Path, std::ios::ate | std::ios::binary);
    const auto pos = m_OpenMode & std::ios::in ? m_Stream.tellg() : m_Stream.tellp();
    if (!m_Stream.good())
      return false;
    length = pos;
    return true;
#endif
  }

  bool FileBase::Seek(Int64 distanceToMove, DWORD moveMethod, UInt64& newPosition)  noexcept {
#ifdef _WIN32
    LONG high = (LONG)(distanceToMove >> 32);
    DWORD low = ::SetFilePointer(m_Handle, (LONG)(distanceToMove & 0xFFFFFFFF), &high, moveMethod);
    if (low == 0xFFFFFFFF)
      if (::GetLastError() != NO_ERROR)
        return false;
    newPosition = (((UInt64)(UInt32)high) << 32) + low;
    return true;
#else
    if (!m_Stream.is_open() || !(m_OpenMode & (std::ios::in | std::ios::out)) ||
        (moveMethod != FILE_BEGIN && moveMethod != FILE_CURRENT && moveMethod != FILE_END))
      return false;
    const auto m = moveMethod == FILE_BEGIN ? std::ios::beg : moveMethod == FILE_CURRENT ? std::ios::cur : std::ios::end;
    if (!m_Stream.good())
      m_Stream.clear();
    m_OpenMode & std::ios::in ? (void) m_Stream.seekg(distanceToMove, m) : (void) m_Stream.seekp(distanceToMove, m);
    const auto pos = m_OpenMode & std::ios::in ? m_Stream.tellg() : m_Stream.tellp();
    if (!m_Stream.good())
      return false;
    newPosition = pos;
    return true;
#endif
  }
  bool FileBase::Seek(UInt64 position, UInt64& newPosition) noexcept {
    return Seek(position, FILE_BEGIN, newPosition);
  }

  bool FileBase::SeekToBegin() noexcept {
    UInt64 newPosition;
    return Seek(0, newPosition);
  }

  bool FileBase::SeekToEnd(UInt64& newPosition) noexcept {
    return Seek(0, FILE_END, newPosition);
  }

#ifdef _WIN32
  bool FileBase::Create(std::filesystem::path const& path, DWORD desiredAccess, DWORD shareMode, DWORD creationDisposition, DWORD flagsAndAttributes) noexcept {
    if (!Close()) {
      return false;
    }

    m_Handle = ::CreateFileW(path.c_str(), desiredAccess, shareMode,
      (LPSECURITY_ATTRIBUTES)NULL, creationDisposition, flagsAndAttributes, (HANDLE)NULL);

    return m_Handle != INVALID_HANDLE_VALUE;
  }
#else
  bool FileBase::Create(std::filesystem::path const& path, std::ios::openmode openMode) noexcept {
    if (!Close()) {
      return false;
    }

    m_Stream.open(m_Path = path, m_OpenMode = openMode | std::ios::binary);
    std::cerr << "FIXME: FileBase::Create '" + path.string() + "'" + std::string(" \e]8;;eclsrc://") + __FILE__ + ":" + std::to_string(__LINE__) + "\a" + __FILE__ + ":" + std::to_string(__LINE__) + "\e]8;;\a\n";
    return m_Stream.good();
  }
#endif

  bool FileBase::GetFileInformation(std::filesystem::path const& path, FileInfo* info) noexcept {
//    // Use FileBase to open/close the file:
//    FileBase file;
//    if (!file.Create(path, 0, FILE_SHARE_READ, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS))
//      return false;
//
//    BY_HANDLE_FILE_INFORMATION finfo;
//    if (!BOOLToBool(GetFileInformationByHandle(file.m_Handle, &finfo))) {
//      return false;
//    }
//
//    *info = FileInfo(path, finfo);
//    return true;
    std::cerr << "FIXME: Not implemented" + std::string(" \e]8;;eclsrc://") + __FILE__ + ":" + std::to_string(__LINE__) + "\a" + __FILE__ + ":" + std::to_string(__LINE__) + "\e]8;;\a\n"; assert(false && "Not implemented");
    return true;
  }

  // FileIn

#ifdef _WIN32
  bool FileIn::Open(std::filesystem::path const& filepath, DWORD shareMode, DWORD creationDisposition, DWORD flagsAndAttributes) noexcept {
    bool res = Create(filepath.c_str(), GENERIC_READ, shareMode, creationDisposition, flagsAndAttributes);
    return res;
  }
  bool FileIn::OpenShared(std::filesystem::path const& filepath, bool shareForWrite) noexcept {
    return Open(filepath, FILE_SHARE_READ | (shareForWrite ? FILE_SHARE_WRITE : 0), OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL);
  }
#endif
  bool FileIn::Open(std::filesystem::path const& filepath) noexcept {
#ifdef _WIN32
    return OpenShared(filepath, false);
#else
    return Create(filepath, std::ios::in);
#endif
  }
  bool FileIn::Read(void* data, UInt32 size, UInt32& processedSize) noexcept {
    processedSize = 0;
    do
    {
      UInt32 processedLoc = 0;
      bool res = ReadPart(data, size, processedLoc);
      processedSize += processedLoc;
      if (!res)
        return false;
      if (processedLoc == 0)
        return true;
      data = (void*)((unsigned char*)data + processedLoc);
      size -= processedLoc;
    } while (size > 0);
    return true;
  }
  bool FileIn::Read1(void* data, UInt32 size, UInt32& processedSize) noexcept {
#ifdef _WIN32
    DWORD processedLoc = 0;
    bool res = BOOLToBool(::ReadFile(m_Handle, data, size, &processedLoc, NULL));
    processedSize = (UInt32)processedLoc;
    return res;
#else
    if (!m_Stream.is_open() || !(m_OpenMode & std::ios::in))
      return false;
    m_Stream.read((char*)data, size);
    processedSize = m_Stream.gcount();
    return !m_Stream.bad();
#endif
  }
  bool FileIn::ReadPart(void* data, UInt32 size, UInt32& processedSize) noexcept {
    if (size > kChunkSizeMax)
      size = kChunkSizeMax;
    return Read1(data, size, processedSize);
  }

  // FileOut

#ifdef _WIN32
  bool FileOut::Open(std::filesystem::path const& fileName, DWORD shareMode, DWORD creationDisposition, DWORD flagsAndAttributes) noexcept {
    return Create(fileName, GENERIC_WRITE, shareMode, creationDisposition, flagsAndAttributes);
  }
#endif

  bool FileOut::Open(std::filesystem::path const& fileName) noexcept {
#ifdef _WIN32
    return Open(fileName, FILE_SHARE_READ, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL);
#else
    return Create(fileName, std::ios::out);
#endif
  }

  bool FileOut::SetTime(const FILETIME* cTime, const FILETIME* aTime, const FILETIME* mTime) noexcept {
#ifdef _WIN32
    return BOOLToBool(::SetFileTime(m_Handle, cTime, aTime, mTime));
#else
    std::cerr << "FIXME: Not implemented FileOut::SetTime" + std::string(" \e]8;;eclsrc://") + __FILE__ + ":" + std::to_string(__LINE__) + "\a" + __FILE__ + ":" + std::to_string(__LINE__) + "\e]8;;\a\n";
    return true;
#endif
  }
  bool FileOut::SetMTime(const FILETIME* mTime) noexcept {
    return SetTime(NULL, NULL, mTime);
  }
  bool FileOut::Write(const void* data, UInt32 size, UInt32& processedSize) noexcept {
    processedSize = 0;
    do
    {
      UInt32 processedLoc = 0;
      bool res = WritePart(data, size, processedLoc);
      processedSize += processedLoc;
      if (!res)
        return false;
      if (processedLoc == 0)
        return true;
      data = (const void*)((const unsigned char*)data + processedLoc);
      size -= processedLoc;
    } while (size > 0);
    return true;
  }

  bool FileOut::SetLength(UInt64 length) noexcept {
    UInt64 newPosition;
    if (!Seek(length, newPosition))
      return false;
    if (newPosition != length)
      return false;
    return SetEndOfFile();
  }
  bool FileOut::SetEndOfFile() noexcept {
#ifdef _WIN32
    return BOOLToBool(::SetEndOfFile(m_Handle));
#else
    if (!m_Stream.is_open() || !(m_OpenMode & std::ios::out))
      return false;
    m_Stream.flush();
    if (!m_Stream.good())
      return false;
    UInt64 pos;
    if (!GetPosition(pos))
      return false;
    std::error_code ec;
    std::filesystem::resize_file(m_Path, pos, ec);
    return !ec;
#endif
  }

  bool FileOut::WritePart(const void* data, UInt32 size, UInt32& processedSize) noexcept {
    if (size > kChunkSizeMax)
      size = kChunkSizeMax;
#ifdef _WIN32
    DWORD processedLoc = 0;
    bool res = BOOLToBool(::WriteFile(m_Handle, data, size, &processedLoc, NULL));
    processedSize = (UInt32)processedLoc;
    return res;
#else
    if (!m_Stream.is_open() || !(m_OpenMode & std::ios::out))
      return false;
    m_Stream.write((char*)data, size);
    processedSize = size;
    return !m_Stream.bad();
#endif
  }

}
