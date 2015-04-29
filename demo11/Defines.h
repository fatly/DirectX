#ifndef __DEFINES_H__
#define __DEFINES_H__

#define HR(x) {hr = (x); if (FAILED(hr)) return hr;}
#define SAFE_RELEASE(x) if ((x) != 0){(x)->Release(); (x) = 0;}
#define SAFE_DELETE(x)  if ((x) != 0){ delete (x); (x) = 0;}
#define SAFE_FREE(x) if ((x) != 0){free(x); (x) = 0;}

#endif