#ifndef export_hpp__
#define export_hpp__

#if defined(_MSC_VER)
#if defined(RYUJIN_EXPORT)
#define RYUJIN_API __declspec(dllexport)
#else
#if defined(RYUJIN_DLL)
#define RYUJIN_API __declspec(dllimport)
#else
#define RYUJIN_API
#endif
#endif
#else
#define RYUJIN_API
#endif

#endif // export_hpp__
