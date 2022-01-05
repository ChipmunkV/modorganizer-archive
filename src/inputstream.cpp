/*
Mod Organizer archive handling

Copyright (C) 2012 Sebastian Herbord, 2020 MO2 Team. All rights reserved.

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

//#include <Unknwn.h>
#include "inputstream.h"

static inline HRESULT ConvertBoolToHRESULT(bool result)
{
  std::cerr << "FIXME: inputstream ConvertBoolToHRESULT" + std::string(" \e]8;;eclsrc://") + __FILE__ + ":" + std::to_string(__LINE__) + "\a" + __FILE__ + ":" + std::to_string(__LINE__) + "\e]8;;\a\n";
#ifdef _WIN32
  if (result) {
    return S_OK;
  }
  DWORD lastError = ::GetLastError();
  if (lastError == 0) {
    return E_FAIL;
  }
  return HRESULT_FROM_WIN32(lastError);
#else
  return result ? S_OK : E_FAIL;
#endif
}

InputStream::InputStream()
{}

InputStream::~InputStream() { }


bool InputStream::Open(std::filesystem::path const& filename)
{
  return m_File.Open(filename);
}

STDMETHODIMP InputStream::Read(void *data, UInt32 size, UInt32 *processedSize)
{
  std::cerr << "FIXME: inputstream Read" + std::string(" \e]8;;eclsrc://") + __FILE__ + ":" + std::to_string(__LINE__) + "\a" + __FILE__ + ":" + std::to_string(__LINE__) + "\e]8;;\a\n";
  UInt32 realProcessedSize;
  bool result =  m_File.Read(data, size, realProcessedSize);

  if (processedSize != nullptr) {
    *processedSize = realProcessedSize;
  }

#ifdef _WIN32
  if (result) {
    return S_OK;
  }

  return HRESULT_FROM_WIN32(::GetLastError());
#else
  return result ? S_OK : E_FAIL;
#endif
}

STDMETHODIMP InputStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition)
{
  UInt64 realNewPosition = offset;
  bool result = m_File.Seek(offset, seekOrigin, realNewPosition);

  if (newPosition) {
    *newPosition = realNewPosition;
  }
  return ConvertBoolToHRESULT(result);
}
