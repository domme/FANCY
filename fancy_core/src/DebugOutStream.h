#ifndef INCLUDE_DEBUGOUTSTREAM_H_
#define INCLUDE_DEBUGOUTSTREAM_H_

#include <Windows.h>
#include <ostream>
#include <sstream>
#include <string>

namespace Fancy {
//---------------------------------------------------------------------------//
  template <class CharT, class TraitsT = std::char_traits<CharT> >
  class basic_dstringbuf : public std::basic_stringbuf<CharT, TraitsT>
  {
  public:
    virtual ~basic_dstringbuf()
    {
      sync();
    }
  //---------------------------------------------------------------------------//
  protected:
    virtual int sync() override
    {
      output_debug_string(str().c_str());
      str(std::basic_string<CharT>());

      return 0;
    }
  //---------------------------------------------------------------------------//
    void output_debug_string(const CharT* pText) {}
  };
//---------------------------------------------------------------------------//
  template<>
  void basic_dstringbuf<char>::output_debug_string(const char* pText)
  {
    OutputDebugStringA(pText);
  }
//---------------------------------------------------------------------------//
  template<>
  void basic_dstringbuf<wchar_t>::output_debug_string(const wchar_t* pText)
  {
    OutputDebugStringW(pText);
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  template <class CharT, class TraitsT = std::char_traits<CharT> >
  class basic_dostream : public std::basic_ostream<CharT, TraitsT>
  {
  public :
    basic_dostream() : std::basic_ostream<CharT, TraitsT>(new basic_dstringbuf<CharT, TraitsT>()) {}
    ~basic_dostream()
    {
      delete rdbuf();
    }
  };

  typedef basic_dostream<char> dostream;
  typedef basic_dostream<wchar_t> wdostream;
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  class DebugOutStream
  {
    public:
      static dostream out;
      static wdostream w_out;

    private:
      DebugOutStream() {}
      ~DebugOutStream() {}
  };
//---------------------------------------------------------------------------//

} // end of namespace Fancy

#endif  // INCLUDE_DEBUGOUTSTREAM_H_