struct MODE_INFO
{
 _UINT  ModeAttributes       __attribute__ ((packed));
 _UCHAR WinAAttributes       __attribute__ ((packed));
 _UCHAR WinBAttributes       __attribute__ ((packed));
 _UINT  WinGranularity       __attribute__ ((packed));
 _UINT  WinSize              __attribute__ ((packed));
 _UINT  WinASegment          __attribute__ ((packed));
 _UINT  WinBSegment          __attribute__ ((packed));
 _ULONG WinFuncPtr           __attribute__ ((packed));
 _UINT  BytesPerScanLine     __attribute__ ((packed));
 _UINT  XResolution          __attribute__ ((packed));
 _UINT  YResolution          __attribute__ ((packed));
 _UCHAR XCharSize            __attribute__ ((packed));
 _UCHAR YCharSize            __attribute__ ((packed));
 _UCHAR NumberOfPlanes       __attribute__ ((packed));
 _UCHAR BitsPerPixel         __attribute__ ((packed));
 _UCHAR NumberOfBanks        __attribute__ ((packed));
 _UCHAR MemoryModel          __attribute__ ((packed));
 _UCHAR BankSize             __attribute__ ((packed));
 _UCHAR NumberOfImagePages   __attribute__ ((packed));
 _UCHAR Reserved_page        __attribute__ ((packed));
 _UCHAR RedMaskSize          __attribute__ ((packed));
 _UCHAR RedMaskPos           __attribute__ ((packed));
 _UCHAR GreenMaskSize        __attribute__ ((packed));
 _UCHAR GreenMaskPos         __attribute__ ((packed));
 _UCHAR BlueMaskSize         __attribute__ ((packed));
 _UCHAR BlueMaskPos          __attribute__ ((packed));
 _UCHAR ReservedMaskSize     __attribute__ ((packed));
 _UCHAR ReservedMaskPos      __attribute__ ((packed));
 _UCHAR DirectColorModeInfo  __attribute__ ((packed));
 _ULONG PhysBasePtr          __attribute__ ((packed));
 _ULONG OffScreenMemOffset   __attribute__ ((packed));
 _UINT  OffScreenMemSize     __attribute__ ((packed));
 _UCHAR Reserved[206]        __attribute__ ((packed));
};

struct VESA_INFO
{
 _UCHAR VESASignature[4]     __attribute__ ((packed));
 _UINT  VESAVersion          __attribute__ ((packed));
 _ULONG OemStringPtr         __attribute__ ((packed));
 _UCHAR Capabilities[4]      __attribute__ ((packed));
 _ULONG VideoModePtr         __attribute__ ((packed));
 _UINT  TotalMemory          __attribute__ ((packed));
 _UINT  OemSoftwareRev       __attribute__ ((packed));
 _ULONG OemVendorNamePtr     __attribute__ ((packed));
 _ULONG OemProductNamePtr    __attribute__ ((packed));
 _ULONG OemProductRevPtr     __attribute__ ((packed));
 _UCHAR Reserved[222]        __attribute__ ((packed));
 _UCHAR OemData[256]         __attribute__ ((packed));
};
