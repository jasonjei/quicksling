
QuickslingShellps.dll: dlldata.obj QuickslingShell_p.obj QuickslingShell_i.obj
	link /dll /out:QuickslingShellps.dll /def:QuickslingShellps.def /entry:DllMain dlldata.obj QuickslingShell_p.obj QuickslingShell_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del QuickslingShellps.dll
	@del QuickslingShellps.lib
	@del QuickslingShellps.exp
	@del dlldata.obj
	@del QuickslingShell_p.obj
	@del QuickslingShell_i.obj
