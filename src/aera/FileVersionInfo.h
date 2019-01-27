#pragma once


class FileVersionInfo
{
public:
  DEFINE_ERROR(fault, "FileVersionInfo::error");

  FileVersionInfo(std::string path)
  {
    init(const_cast<char*>(path.c_str()));
  }

  void init(LPSTR fname)
  {
    bufsize=GetFileVersionInfoSize(fname, NULL);
    debug::Assert<fault>(bufsize, HERE); 

    buffer=new char[bufsize];
    BOOL ret=GetFileVersionInfo(fname, NULL, bufsize, buffer);
    debug::Assert<fault>(ret, HERE);
  }

  ~FileVersionInfo()
  {
    delete[] buffer;
  }

  std::string operator[](std::string path)
  {
    char* lpbuf=""; UINT strs;

    BOOL ret=VerQueryValue(buffer, const_cast<char*>(("\\StringFileInfo\\040904B0\\"+path).c_str()), (LPVOID*)&lpbuf, &strs);
    debug::Assert<fault>(ret, HERE);
    return std::string(lpbuf);
  }

  VS_FIXEDFILEINFO QueryValue()
  {
    UINT  szResult;
    VS_FIXEDFILEINFO *result;
    BOOL ret=VerQueryValue(buffer, "\\", (LPVOID*)&result, &szResult);
    debug::Assert<fault>(ret, HERE);
    return *result;
  }

  LPVOID buffer;
  DWORD  bufsize;
};

