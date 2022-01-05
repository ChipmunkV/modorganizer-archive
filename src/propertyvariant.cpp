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

#include "propertyvariant.h"

#ifdef _WIN32
#include <guiddef.h>
#else
#include "Common/MyInitGuid.h"
#include <locale>
#include <codecvt>
#include <string>
#endif

#include <stdint.h>
#include <string>
#include <stdexcept>
#include <cassert> // UNUSED
#include <iostream> // UNUSED

// {23170F69-40C1-278A-0000-000000050000}
DEFINE_GUID(IID_IProgress,
        0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00);

// {23170F69-40C1-278A-0000-000300030000}
DEFINE_GUID(IID_IInStream,
        0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00);

// {23170F69-40C1-278A-0000-000300040000}
DEFINE_GUID(IID_IOutStream,
        0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x03, 0x00, 0x04, 0x00, 0x00);

// {23170F69-40C1-278A-0000-000500100000}
DEFINE_GUID(IID_ICryptoGetTextPassword,
        0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x05, 0x00, 0x10, 0x00, 0x00);

// {23170F69-40C1-278A-0000-000600100000}
DEFINE_GUID(IID_IArchiveOpenCallback,
        0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x10, 0x00, 0x00);

// {23170F69-40C1-278A-0000-000600200000}
DEFINE_GUID(IID_IArchiveExtractCallback,
        0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x20, 0x00, 0x00);

// {23170F69-40C1-278A-0000-000600300000}
DEFINE_GUID(IID_IArchiveOpenVolumeCallback,
        0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x30, 0x00, 0x00);

// {23170F69-40C1-278A-0000-000600500000}
DEFINE_GUID(IID_IArchiveOpenSetSubArchiveName,
        0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x50, 0x00, 0x00);

// {23170F69-40C1-278A-0000-000600600000}
DEFINE_GUID(IID_IInArchive,
        0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x60, 0x00, 0x00);

#ifndef _WIN32
typedef UINT32 CBstrSizeType;
#define k_BstrSize_Max 0xFFFFFFFF

static inline void *AllocateForBSTR(size_t cb) { return ::malloc(cb); }
static inline void FreeForBSTR(void *pv) { ::free(pv);}

UINT SysStringLen(BSTR bstr)
{
  if (!bstr)
    return 0;
  return *((CBstrSizeType *)bstr - 1) / sizeof(OLECHAR);
}

void SysFreeString(BSTR bstr)
{
  if (bstr)
    FreeForBSTR((CBstrSizeType *)bstr - 1);
}

UINT SysStringByteLen(BSTR bstr)
{
  if (!bstr)
    return 0;
  return *((CBstrSizeType *)bstr - 1);
}

BSTR SysAllocStringLen(const OLECHAR *s, UINT len)
{
  if (len >= (k_BstrSize_Max - sizeof(OLECHAR) - sizeof(CBstrSizeType)) / sizeof(OLECHAR))
    return NULL;

  UINT size = len * sizeof(OLECHAR);
  void *p = AllocateForBSTR(size + sizeof(CBstrSizeType) + sizeof(OLECHAR));
  if (!p)
    return NULL;
  *(CBstrSizeType *)p = (CBstrSizeType)size;
  BSTR bstr = (BSTR)((CBstrSizeType *)p + 1);
  if (s)
    memcpy(bstr, s, size);
  bstr[len] = 0;
  return bstr;
}

BSTR SysAllocString(const OLECHAR *s)
{
  if (!s)
    return 0;
  const OLECHAR *s2 = s;
  while (*s2 != 0)
    s2++;
  return SysAllocStringLen(s, (UINT)(s2 - s));
}

HRESULT VariantClear(VARIANTARG *prop)
{
  if (prop->vt == VT_BSTR)
    SysFreeString(prop->bstrVal);
  prop->vt = VT_EMPTY;
  return S_OK;
}
#endif

PropertyVariant::PropertyVariant()
{
  PropVariantInit(this);
}

PropertyVariant::~PropertyVariant()
{
  clear();
}

void PropertyVariant::clear()
{
#ifdef _WIN32
  PropVariantClear(this);
#else
  VariantClear(this);
#endif
}

//Arguably the behviours for empty here are wrong.
template <> PropertyVariant::operator bool() const
{
  switch (vt)
  {
    case VT_EMPTY:
      return false;

    case VT_BOOL:
      return boolVal != VARIANT_FALSE;

    default:
      throw std::runtime_error("Property is not a bool");
  }
}

template <> PropertyVariant::operator uint64_t() const
{
  switch (vt)
  {
    case VT_EMPTY:
      return 0;

    case VT_UI1:
      return bVal;

    case VT_UI2:
      return uiVal;

    case VT_UI4:
      return ulVal;

    case VT_UI8:
      return static_cast<uint64_t>(uhVal.QuadPart);

    default:
      throw std::runtime_error("Property is not an unsigned integer");
  }
}

#ifndef _WIN32
template <> PropertyVariant::operator unsigned long long() const
{
  switch (vt)
  {
    case VT_EMPTY:
      return 0;

    case VT_UI1:
      return bVal;

    case VT_UI2:
      return uiVal;

    case VT_UI4:
      return ulVal;

    case VT_UI8:
      return static_cast<unsigned long long>(uhVal.QuadPart);

    default:
      throw std::runtime_error("Property is not an unsigned integer");
  }
}
#endif


template <> PropertyVariant::operator uint32_t() const
{
  switch (vt)
  {
    case VT_EMPTY:
      return 0;

    case VT_UI1:
      return bVal;

    case VT_UI2:
      return uiVal;

    case VT_UI4:
      return ulVal;

    default:
      throw std::runtime_error("Property is not an unsigned integer");
  }
}


template <> PropertyVariant::operator std::wstring() const
{
  switch (vt)
  {
    case VT_EMPTY:
      return L"";

    case VT_BSTR:
      return std::wstring(bstrVal, ::SysStringLen(bstrVal));

    default:
      throw std::runtime_error("Property is not a string");
  }
}

//This is what he does, though it looks rather a strange use of the property
template <> PropertyVariant::operator std::string() const
{
  switch (vt)
  {
    case VT_EMPTY:
      return "";

    case VT_BSTR:
      //If he can do a memcpy, I can do a reinterpret case
      return std::string(reinterpret_cast<char const *>(bstrVal), ::SysStringByteLen(bstrVal));

    default:
      throw std::runtime_error("Property is not a string");
  }
}

//This is what he does, though it looks rather a strange use of the property
template <> PropertyVariant::operator GUID() const
{
  switch (vt)
  {
    case VT_BSTR:
      //He did a cast too!
      return *reinterpret_cast<const GUID *>(bstrVal);

    default:
      throw std::runtime_error("Property is not a GUID (string)");
  }
}

template <> PropertyVariant::operator FILETIME() const
{
  switch (vt)
  {
    case VT_FILETIME:
      return filetime;

    default:
      throw std::runtime_error("Property is not a file time");
  }
}

//Assignments
template <> PropertyVariant& PropertyVariant::operator=(std::wstring const &str)
{
  clear();
  vt = VT_BSTR;
  bstrVal = ::SysAllocString(str.c_str());
  if (bstrVal == NULL) {
    throw std::bad_alloc();
  }
  return *this;
}

template <> PropertyVariant& PropertyVariant::operator=(std::string const &str)
{
  std::cerr << "FIXME: PropertyVariant::operator= '" + str + "'" + std::string(" \e]8;;eclsrc://") + __FILE__ + ":" + std::to_string(__LINE__) + "\a" + __FILE__ + ":" + std::to_string(__LINE__) + "\e]8;;\a\n";
  return *this = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(str);
}

template <> PropertyVariant& PropertyVariant::operator=(bool const& n)
{
  clear();
  vt = VT_BOOL;
  boolVal = n ? VARIANT_TRUE : VARIANT_FALSE;
  return *this;
}

template <> PropertyVariant& PropertyVariant::operator=(FILETIME const& n)
{
  clear();
  vt = VT_FILETIME;
  filetime = n;
  return *this;
}

template <> PropertyVariant& PropertyVariant::operator=(uint32_t const& n)
{
  clear();
  vt = VT_UI4;
  ulVal = n;
  return *this;
}

template <> PropertyVariant& PropertyVariant::operator=(uint64_t const &n)
{
  clear();
  vt = VT_UI8;
  uhVal.QuadPart = n;
  return *this;
}

template <> PropertyVariant& PropertyVariant::operator=(unsigned long long const &n)
{
  clear();
  vt = VT_UI8;
  uhVal.QuadPart = n;
  return *this;
}
