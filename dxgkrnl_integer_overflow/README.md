### About Vulnerability

----
#### Crash Dump

```
kd> g 
KDTARGET: Refreshing KD connection

*** Fatal System Error: 0x00000050
(0xFFFFB906D428D880,0x0000000000000002,0xFFFFF802125744AC,0x000000000000000C)

Driver at fault:
***   dxgkrnl.sys - Address FFFFF802125744AC base at FFFFF80212550000, DateStamp c9ddfd3f

PAGE_FAULT_IN_NONPAGED_AREA (50)
Invalid system memory was referenced.  This cannot be protected by try-except.
Typically the address is just plain bad or it is pointing at freed memory.
Arguments:
Arg1: ffffb906d428d880, memory referenced.
Arg2: 0000000000000002, value 0 = read operation, 1 = write operation.
Arg3: fffff802125744ac, If non-zero, the instruction address which referenced the bad memory
address.
Arg4: 000000000000000c, (reserved)
```
---
#### Stack Trace
```
STACK_TEXT:
ffffed88`9e385f38 fffff802`87683472 : ffffb906`d428d880 00000000`00000003 ffffed88`9e3860a0 fffff802`87541450 : nt!DbgBreakPointWithStatus
ffffed88`9e385f40 fffff802`87682bf3 : 00000000`00000003 ffffed88`9e3860a0 fffff802`875cd3e0 ffffed88`9e3865f0 : nt!KiBugCheckDebugBreak+0x12
ffffed88`9e385fa0 fffff802`875b97e7 : 0010001f`00000000 00000000`000004f7 ffffed88`9e3869d0 ffffb905`000004f7 : nt!KeBugCheck2+0x943
ffffed88`9e3866c0 fffff802`875e8810 : 00000000`00000050 ffffb906`d428d880 00000000`00000002 ffffed88`9e3869d0 : nt!KeBugCheckEx+0x107
ffffed88`9e386700 fffff802`874a0d19 : ffffed88`9e386ab0 00000000`00000002 ffffed88`9e3869d0 ffffb906`d428d880 : nt!MiSystemFault+0x1a43b0
ffffed88`9e386840 fffff802`875c728b : 00000000`00000000 fffff802`878267c0 ffffed88`9e386c10 00000000`00000000 : nt!MmAccessFault+0x199
ffffed88`9e3869d0 fffff802`125744ac : fffff802`1277daca 00000000`00000000 ffffed88`9e386cc0 ffffb905`d59aca00 : nt!KiPageFault+0x30b
ffffed88`9e386b68 fffff802`1277daca : 00000000`00000000 ffffed88`9e386cc0 ffffb905`d59aca00 fffff802`1255a6e8 : dxgkrnl!memcpy+0x2ec
ffffed88`9e386b70 fffff802`1277de1c : ffffb905`d428d890 00000000`00000000 ffff948f`9bc246a0 00000000`00000001 : dxgkrnl!ReadPresentDirtyRectsData+0x136
ffffed88`9e386bc0 fffff802`1277d4d5 : ffffed88`9e387040 ffffed88`9e3878b0 00000000`00000000 ffff948f`9d63fe30 : dxgkrnl!DXGCONTEXT::SubmitPresentMultiPlaneOverlays3+0x204
ffffed88`9e386ef0 fffff802`12780170 : ffffffff`c000000d 00000000`00000001 00000000`00000001 ffffed88`9e387938 : dxgkrnl!DXGCONTEXT::PresentMultiPlaneOverlay3+0x1129
ffffed88`9e387810 ffffeae3`2c483191 : 00000000`00000000 ffff948f`9d63fe30 ffffb905`d66d4080 00000188`98a529cd : dxgkrnl!DxgkPresentMultiPlaneOverlay3+0x600
ffffed88`9e387a50 fffff802`875ca6b3 : 00000007`e72fed70 ffffb905`d66d4080 00000188`98a529cd 00000000`00000020 : win32kbase!NtGdiDdDDIPresentMultiPlaneOverlay3+0x11
```
---
***Vulnerability code***

```
signed __int64 __fastcall ReadPresentDirtyRectsData(const struct DXGADAPTER *a1, unsigned int a2, struct _D3DKMT_MULTIPLANE_OVERLAY3 **a3, struct CRefCountedBuffer **a4)
{
for ( i = 0i64; (unsigned int)i < v6; i = (unsigned int)(i + 1) )
{
v10 = a3[i];
if ( *((_DWORD *)v10 + 1) & 1 )
{
v11 = *(_DWORD *)(*((_QWORD *)v10 + 6) + 60i64);
if ( v11 )
{
v7 += v11;
v8 = (unsigned int)(v8 + 1);
}
}
}
if ( v7 && (_DWORD)v8 )
{
v12 = 16 * (v8 + v7) + 8;
v13 = operator_new__((unsigned int)(16 * (v8 + v7) + 16), 0x4B677844u, (POOL_TYPE)512); //integer overflow
v14 = (struct CRefCountedBuffer *)v13;
if ( !v13 )
{
LODWORD(v15) = WdLogNewEntry5_WdLowResource();
*(_QWORD *)(v15 + 24) = 1415i64;
WdLogEvent5_WdLowResource(v15);
return 3221225495i64;
}
……
memmove(v18, v23, v22); //trigger crash

}
return 0i64;
}
```
---
***After patch***

```
for ( i = 0i64; (unsigned int)i < v6; i = (unsigned int)(i + 1) )
  {
    v10 = v5[i]; //
    v11 = *((_QWORD *)v10 + 6);
    v12 = *(_DWORD *)(v11 + 0x3C);
    if ( v12 > 0xFFF )
    {
      v13 = WdLogNewEntry5_WdWarning(i);
      *(_QWORD *)(v13 + 24) = *(unsigned int *)(v11 + 60);
      WdLogEvent5_WdWarning(v13);
      return 3221225485i64;
    }
    if ( *((_DWORD *)v10 + 1) & 1 )
    {
      if ( v12 )
      {
        v7 += v12;
        v8 = (unsigned int)(v8 + 1);
      }
    }
  }
  if ( v7 && (_DWORD)v8 )
  {
    v15 = 16 * (v8 + v7) + 8;
    v16 = (struct CRefCountedBuffer *)operator new[](
                                        (unsigned int)(16 * (v8 + v7) + 24),
                                        0x4B677844u,
                                        (enum _POOL_TYPE)512);
```
---
variable v7 is D3DKMT_PRESENT_MULTIPLANE_OVERLAY3->PresentPlaneCount(count of D3DKMT_MULTIPLANE_OVERLAY2), variable v8 is D3DKMT_PRESENT_MULTIPLANE_OVERLAY3->D3DKMT_MULTIPLANE_OVERLAY2->D3DKMT_MULTIPLANE_OVERLAY_ATTRIBUTES2->DirtyRectCount, function ReadPresentDirtyRectsData didn't check these two variables are in correct range, it will trigger integer overflow if I set v7 to 0x1 and set v8 to 0xfffffffe.

After I review the code after patch, I found that there may be another way to bypass the patch first, I think although function check D3DKMT_PRESENT_MULTIPLANE_OVERLAY3->D3DKMT_MULTIPLANE_OVERLAY2->D3DKMT_MULTIPLANE_OVERLAY_ATTRIBUTES2->DirtyRectCount if it's lager than 0xFFF, but it use a loop to add variable, if I can set D3DKMT_PRESENT_MULTIPLANE_OVERLAY3->PresentPlaneCount to a large count, it may trigger integer overflow finally.

But finally it fail because there is a check in function CapturePresentMultiPlaneOverlayArgs3.

```
  if ( (unsigned int)(HIDWORD(v12) - 1) > 9 ) // check pPresentPlaneCount
  {
    v58 = (_QWORD *)WdLogNewEntry5_WdError(a1, a2);
    v61 = HIDWORD(v73[1]);
    v58[3] = HIDWORD(v73[1]);
    v17 = -1073741811;
    v58[4] = -1073741811i64;
    v58[5] = PsGetCurrentProcess(v61, v62);
    goto LABEL_67;
  }
```
---
#### Time Line

***Jul 16 2018:*** Vulnerability reported
***Jul 17 2018:*** Microsoft reproduced
***Sep 11 2018:*** Patch release
***Oct 10 2018:*** Bounty awarded

