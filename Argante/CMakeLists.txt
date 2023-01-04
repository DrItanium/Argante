cmake_minimum_required(VERSION 3.10)
project(hitagimon VERSION 0.1)
enable_language(CXX C)
file(READ kernel/.argbuild BUILDNUMBER)
execute_process(COMMAND whoami OUTPUT_VARIABLE CURRENT_USER OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND uname -n OUTPUT_VARIABLE MACHINE_NAME OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND date "+%A, %d %B %Y, %H:%M" OUTPUT_VARIABLE CURRENT_DATE_TIME OUTPUT_STRIP_TRAILING_WHITESPACE)
set(IDSTR "${CURRENT_USER}@${MACHINE_NAME} on ${CURRENT_DATE_TIME}")
string(STRIP ${BUILDNUMBER} BUILDNUMBER)
string(STRIP ${IDSTR} IDSTR)
add_compile_definitions(BUILD=${BUILDNUMBER} IDSTR="${IDSTR}")
add_custom_command(
	OUTPUT include/autogen-debug.h
	COMMAND include/autogen.sh
	DEPENDS include/autogen.sh modules/syscall.h
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
# target_include_directories(hitagicortex PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_command(
	OUTPUT compiler/autogen.h
	COMMAND compiler/autogen.sh
	DEPENDS compiler/autogen.sh modules/syscall.h include/exception.h
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})


add_executable(agtc 
	include/config.h
	include/exception.h
	modules/syscall.h
	compiler/autogen.h
	include/autogen-debug.h
	compiler/agtc.c)

target_include_directories(agtc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# @todo add support for generating the IDSTR value

add_executable(agrante 
	kernel/bcode.c
	kernel/console.c
	kernel/manager.c
	kernel/module.c
	kernel/task.c
	kernel/acman.c
	kernel/debugger.c
	kernel/cmd.c
	kernel/main-boot.c
	include/acman.h
	include/bcode.h
	include/cmd.h
	include/config.h
	include/console.h
	include/debugger.h
	include/evaluate.h
	include/language.h
	include/manager.h
	include/memory.h
	include/module.h
	include/task.h
)
target_include_directories(agrante PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(display  SHARED
	include/config.h
	include/task.h
	include/bcode.h
	include/module.h
	include/memory.h
	include/console.h
	include/syscall.h
	include/acman.h
	modules/display.c)

add_library(fs SHARED
	include/config.h
	include/task.h
	include/bcode.h
	include/module.h
	include/memory.h
	include/console.h
	include/syscall.h
	include/acman.h
	modules/fs.c)

add_library(ipc SHARED
	include/config.h
	include/task.h
	include/bcode.h
	include/module.h
	include/memory.h
	include/console.h
	include/syscall.h
	include/acman.h
	modules/ipc.c)

add_library(network SHARED
	include/config.h
	include/task.h
	include/bcode.h
	include/module.h
	include/memory.h
	include/console.h
	include/syscall.h
	include/exception.h
	include/acman.h
	modules/network.c)

add_library(packet SHARED
	include/config.h
	include/task.h
	include/bcode.h
	include/module.h
	include/memory.h
	include/console.h
	include/syscall.h
	include/acman.h
	include/exception.h
	modules/packet.c)
add_library(advmem SHARED
	include/config.h
	include/task.h
	include/bcode.h
	include/module.h
	include/memory.h
	include/console.h
	include/syscall.h
	include/acman.h
	include/exception.h
	modules/advmem.c)
add_library(access SHARED
	include/config.h
	include/task.h
	include/bcode.h
	include/module.h
	include/memory.h
	include/console.h
	include/syscall.h
	include/acman.h
	include/exception.h
	modules/access.c)
add_library(locallib SHARED
	include/config.h
	include/task.h
	include/bcode.h
	include/module.h
	include/memory.h
	include/console.h
	include/syscall.h
	include/acman.h
	include/exception.h
	modules/locallib.c)

add_library(math SHARED
	include/config.h
	include/task.h
	include/bcode.h
	include/module.h
	include/memory.h
	include/console.h
	include/syscall.h
	include/acman.h
	include/exception.h
	modules/math.c)

target_include_directories(display PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(fs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(ipc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(network PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(advmem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(access PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(locallib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(math PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(packet PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)


# add_library(hitagisys
# 		# disabled entries are because mon960 provides the implementations for them
# 		#sys/brk.cc
# 		#sys/fstat.cc
# 		#sys/isatty.cc
# 		sys/lseek.cc
# 		sys/write.cc
# 		sys/read.cc
# 		sys/open.cc
# 		#sys/kill.cc
# 		sys/setitimer.cc
# 		sys/close.cc
# 		sys/gettimeofday.cc
# 		#sys/access.cc
# 		#sys/getpid.cc
# 		sys/_exit.cc
# 		sys/link.cc
# 		sys/unlink.cc sys/linkage.cc)
# add_library(hitagicortex
# 		cortex/IODevice.cc
# 		cortex/IODevice.h
# 		cortex/ChipsetInteract.h
# 		cortex/Interrupts.cc
# 		cortex/Interrupts.h
# 		cortex/EnvironmentInterface.cc
# 		cortex/EnvironmentInterface.h
# 		cortex/Faults.cc
# 		cortex/Faults.h
# 		cortex/IAC.h
# 		cortex/IAC.cc
# 		cortex/IACImplementation.s
# 		cortex/ModernCpp.h
# 		cortex/Types.h cortex/SystemCounter.h cortex/SystemCounter.cc)
# add_library(hitagiclips
# 		clips/agenda.c
# 		clips/agenda.h
# 		clips/analysis.c
# 		clips/analysis.h
# 		clips/argacces.c
# 		clips/argacces.h
# 		clips/bload.c
# 		clips/bload.h
# 		clips/bmathfun.c
# 		clips/bmathfun.h
# 		clips/bsave.c
# 		clips/bsave.h
# 		clips/classcom.c
# 		clips/classcom.h
# 		clips/classexm.c
# 		clips/classexm.h
# 		clips/classfun.c
# 		clips/classfun.h
# 		clips/classinf.c
# 		clips/classinf.h
# 		clips/classini.c
# 		clips/classini.h
# 		clips/classpsr.c
# 		clips/classpsr.h
# 		clips/clips.h
# 		clips/clsltpsr.c
# 		clips/clsltpsr.h
# 		clips/commline.c
# 		clips/commline.h
# 		clips/conscomp.c
# 		clips/conscomp.h
# 		clips/constant.h
# 		clips/constrct.c
# 		clips/constrct.h
# 		clips/constrnt.c
# 		clips/constrnt.h
# 		clips/crstrtgy.c
# 		clips/crstrtgy.h
# 		clips/cstrcbin.c
# 		clips/cstrcbin.h
# 		clips/cstrccmp.h
# 		clips/cstrccom.c
# 		clips/cstrccom.h
# 		clips/cstrcpsr.c
# 		clips/cstrcpsr.h
# 		clips/cstrnbin.c
# 		clips/cstrnbin.h
# 		clips/cstrnchk.c
# 		clips/cstrnchk.h
# 		clips/cstrncmp.c
# 		clips/cstrncmp.h
# 		clips/cstrnops.c
# 		clips/cstrnops.h
# 		clips/cstrnpsr.c
# 		clips/cstrnpsr.h
# 		clips/cstrnutl.c
# 		clips/cstrnutl.h
# 		clips/default.c
# 		clips/default.h
# 		clips/defins.c
# 		clips/defins.h
# 		clips/developr.c
# 		clips/developr.h
# 		clips/dffctbin.c
# 		clips/dffctbin.h
# 		clips/dffctbsc.c
# 		clips/dffctbsc.h
# 		clips/dffctcmp.c
# 		clips/dffctcmp.h
# 		clips/dffctdef.c
# 		clips/dffctdef.h
# 		clips/dffctpsr.c
# 		clips/dffctpsr.h
# 		clips/dffnxbin.c
# 		clips/dffnxbin.h
# 		clips/dffnxcmp.c
# 		clips/dffnxcmp.h
# 		clips/dffnxexe.c
# 		clips/dffnxexe.h
# 		clips/dffnxfun.c
# 		clips/dffnxfun.h
# 		clips/dffnxpsr.c
# 		clips/dffnxpsr.h
# 		clips/dfinsbin.c
# 		clips/dfinsbin.h
# 		clips/dfinscmp.c
# 		clips/dfinscmp.h
# 		clips/drive.c
# 		clips/drive.h
# 		clips/emathfun.c
# 		clips/emathfun.h
# 		clips/engine.c
# 		clips/engine.h
# 		clips/entities.h
# 		clips/envrnbld.c
# 		clips/envrnbld.h
# 		clips/envrnmnt.c
# 		clips/envrnmnt.h
# 		clips/evaluatn.c
# 		clips/evaluatn.h
# 		clips/expressn.c
# 		clips/expressn.h
# 		clips/exprnbin.c
# 		clips/exprnbin.h
# 		clips/exprnops.c
# 		clips/exprnops.h
# 		clips/exprnpsr.c
# 		clips/exprnpsr.h
# 		clips/extnfunc.c
# 		clips/extnfunc.h
# 		clips/factbin.c
# 		clips/factbin.h
# 		clips/factbld.c
# 		clips/factbld.h
# 		clips/factcmp.c
# 		clips/factcmp.h
# 		clips/factcom.c
# 		clips/factcom.h
# 		clips/factfile.c
# 		clips/factfile.h
# 		clips/factfun.c
# 		clips/factfun.h
# 		clips/factgen.c
# 		clips/factgen.h
# 		clips/facthsh.c
# 		clips/facthsh.h
# 		clips/factlhs.c
# 		clips/factlhs.h
# 		clips/factmch.c
# 		clips/factmch.h
# 		clips/factmngr.c
# 		clips/factmngr.h
# 		clips/factprt.c
# 		clips/factprt.h
# 		clips/factqpsr.c
# 		clips/factqpsr.h
# 		clips/factqury.c
# 		clips/factqury.h
# 		clips/factrete.c
# 		clips/factrete.h
# 		clips/factrhs.c
# 		clips/factrhs.h
# 		clips/filecom.c
# 		clips/filecom.h
# 		clips/filertr.c
# 		clips/filertr.h
# 		clips/fileutil.c
# 		clips/fileutil.h
# 		clips/generate.c
# 		clips/generate.h
# 		clips/genrcbin.c
# 		clips/genrcbin.h
# 		clips/genrccmp.c
# 		clips/genrccmp.h
# 		clips/genrccom.c
# 		clips/genrccom.h
# 		clips/genrcexe.c
# 		clips/genrcexe.h
# 		clips/genrcfun.c
# 		clips/genrcfun.h
# 		clips/genrcpsr.c
# 		clips/genrcpsr.h
# 		clips/globlbin.c
# 		clips/globlbin.h
# 		clips/globlbsc.c
# 		clips/globlbsc.h
# 		clips/globlcmp.c
# 		clips/globlcmp.h
# 		clips/globlcom.c
# 		clips/globlcom.h
# 		clips/globldef.c
# 		clips/globldef.h
# 		clips/globlpsr.c
# 		clips/globlpsr.h
# 		clips/immthpsr.c
# 		clips/immthpsr.h
# 		clips/incrrset.c
# 		clips/incrrset.h
# 		clips/inherpsr.c
# 		clips/inherpsr.h
# 		clips/inscom.c
# 		clips/inscom.h
# 		clips/insfile.c
# 		clips/insfile.h
# 		clips/insfun.c
# 		clips/insfun.h
# 		clips/insmngr.c
# 		clips/insmngr.h
# 		clips/insmoddp.c
# 		clips/insmoddp.h
# 		clips/insmult.c
# 		clips/insmult.h
# 		clips/inspsr.c
# 		clips/inspsr.h
# 		clips/insquery.c
# 		clips/insquery.h
# 		clips/insqypsr.c
# 		clips/insqypsr.h
# 		clips/iofun.c
# 		clips/iofun.h
# 		clips/lgcldpnd.c
# 		clips/lgcldpnd.h
# 		clips/main.c
# 		clips/match.h
# 		clips/memalloc.c
# 		clips/memalloc.h
# 		clips/miscfun.c
# 		clips/miscfun.h
# 		clips/modulbin.c
# 		clips/modulbin.h
# 		clips/modulbsc.c
# 		clips/modulbsc.h
# 		clips/modulcmp.c
# 		clips/modulcmp.h
# 		clips/moduldef.c
# 		clips/moduldef.h
# 		clips/modulpsr.c
# 		clips/modulpsr.h
# 		clips/modulutl.c
# 		clips/modulutl.h
# 		clips/msgcom.c
# 		clips/msgcom.h
# 		clips/msgfun.c
# 		clips/msgfun.h
# 		clips/msgpass.c
# 		clips/msgpass.h
# 		clips/msgpsr.c
# 		clips/msgpsr.h
# 		clips/multifld.c
# 		clips/multifld.h
# 		clips/multifun.c
# 		clips/multifun.h
# 		clips/network.h
# 		clips/objbin.c
# 		clips/objbin.h
# 		clips/objcmp.c
# 		clips/objcmp.h
# 		clips/object.h
# 		clips/objrtbin.c
# 		clips/objrtbin.h
# 		clips/objrtbld.c
# 		clips/objrtbld.h
# 		clips/objrtcmp.c
# 		clips/objrtcmp.h
# 		clips/objrtfnx.c
# 		clips/objrtfnx.h
# 		clips/objrtgen.c
# 		clips/objrtgen.h
# 		clips/objrtmch.c
# 		clips/objrtmch.h
# 		clips/parsefun.c
# 		clips/parsefun.h
# 		clips/pattern.c
# 		clips/pattern.h
# 		clips/pprint.c
# 		clips/pprint.h
# 		clips/prccode.c
# 		clips/prccode.h
# 		clips/prcdrfun.c
# 		clips/prcdrfun.h
# 		clips/prcdrpsr.c
# 		clips/prcdrpsr.h
# 		clips/prdctfun.c
# 		clips/prdctfun.h
# 		clips/prntutil.c
# 		clips/prntutil.h
# 		clips/proflfun.c
# 		clips/proflfun.h
# 		clips/reorder.c
# 		clips/reorder.h
# 		clips/reteutil.c
# 		clips/reteutil.h
# 		clips/retract.c
# 		clips/retract.h
# 		clips/router.c
# 		clips/router.h
# 		clips/rulebin.c
# 		clips/rulebin.h
# 		clips/rulebld.c
# 		clips/rulebld.h
# 		clips/rulebsc.c
# 		clips/rulebsc.h
# 		clips/rulecmp.c
# 		clips/rulecmp.h
# 		clips/rulecom.c
# 		clips/rulecom.h
# 		clips/rulecstr.c
# 		clips/rulecstr.h
# 		clips/ruledef.c
# 		clips/ruledef.h
# 		clips/ruledlt.c
# 		clips/ruledlt.h
# 		clips/rulelhs.c
# 		clips/rulelhs.h
# 		clips/rulepsr.c
# 		clips/rulepsr.h
# 		clips/scanner.c
# 		clips/scanner.h
# 		clips/setup.h
# 		clips/sortfun.c
# 		clips/sortfun.h
# 		clips/strngfun.c
# 		clips/strngfun.h
# 		clips/strngrtr.c
# 		clips/strngrtr.h
# 		clips/symblbin.c
# 		clips/symblbin.h
# 		clips/symblcmp.c
# 		clips/symblcmp.h
# 		clips/symbol.c
# 		clips/symbol.h
# 		clips/sysdep.c
# 		clips/sysdep.h
# 		clips/textpro.c
# 		clips/textpro.h
# 		clips/tmpltbin.c
# 		clips/tmpltbin.h
# 		clips/tmpltbsc.c
# 		clips/tmpltbsc.h
# 		clips/tmpltcmp.c
# 		clips/tmpltcmp.h
# 		clips/tmpltdef.c
# 		clips/tmpltdef.h
# 		clips/tmpltfun.c
# 		clips/tmpltfun.h
# 		clips/tmpltlhs.c
# 		clips/tmpltlhs.h
# 		clips/tmpltpsr.c
# 		clips/tmpltpsr.h
# 		clips/tmpltrhs.c
# 		clips/tmpltrhs.h
# 		clips/tmpltutl.c
# 		clips/tmpltutl.h
# 		clips/userdata.c
# 		clips/userdata.h
# 		clips/userfunctions.c
# 		clips/usrsetup.h
# 		clips/utility.c
# 		clips/utility.h
# 		clips/watch.c
# 		clips/watch.h
# 		clips/MonitorExtensions.cc clips/MonitorExtensions.h)
# add_executable(hitagimon
# 		boot/sx_init.s # this must ALWAYS be first!
# 		boot/f_table.s
# 		boot/i_handle.s
# 		boot/i_table.s
# 		boot/f_handle.cc
# 		boot/macros.s
# 		boot/ISR.cc
# 		hitagimain.cc
# 		)
# 
# #(add_library(doom
# #		# doom source code
# #		doom/am_map.h
# #		doom/d_englsh.h
# #		doom/d_event.h
# #		doom/d_french.h
# #		doom/d_items.h
# #		doom/d_main.h
# #		doom/d_net.h
# #		doom/doomdata.h
# #		doom/doomdef.h
# #		doom/doomstat.h
# #		doom/doomtype.h
# #		doom/d_player.h
# #		doom/dstrings.h
# #		doom/d_textur.h
# #		doom/d_think.h
# #		doom/d_ticcmd.h
# #		doom/f_finale.h
# #		doom/f_wipe.h
# #		doom/g_game.h
# #		doom/hu_lib.h
# #		doom/hu_stuff.h
# #		doom/i_net.h
# #		doom/info.h
# #		doom/i_sound.h
# #		doom/i_system.h
# #		doom/i_video.h
# #		doom/m_argv.h
# #		doom/m_bbox.h
# #		doom/m_cheat.h
# #		doom/m_fixed.h
# #		doom/m_menu.h
# #		doom/m_misc.h
# #		doom/m_random.h
# #		doom/m_swap.h
# #		doom/p_inter.h
# #		doom/p_local.h
# #		doom/p_mobj.h
# #		doom/p_pspr.h
# #		doom/p_saveg.h
# #		doom/p_setup.h
# #		doom/p_spec.h
# #		doom/p_tick.h
# #		doom/r_bsp.h
# #		doom/r_data.h
# #		doom/r_defs.h
# #		doom/r_draw.h
# #		doom/r_local.h
# #		doom/r_main.h
# #		doom/r_plane.h
# #		doom/r_segs.h
# #		doom/r_sky.h
# #		doom/r_state.h
# #		doom/r_things.h
# #		doom/sounds.h
# #		doom/s_sound.h
# #		doom/st_lib.h
# #		doom/st_stuff.h
# #		doom/tables.h
# #		doom/v_video.h
# #		doom/wi_stuff.h
# #		doom/w_wad.h
# #		doom/z_zone.h
# #		doom/am_map.c
# #		doom/d_items.c
# #		doom/d_main.c
# #		doom/d_net.c
# #		doom/doomdef.c
# #		doom/doomstat.c
# #		doom/dstrings.c
# #		doom/f_finale.c
# #		doom/f_wipe.c
# #		doom/g_game.c
# #		doom/hu_lib.c
# #		doom/hu_stuff.c
# #		doom/i_main.c
# #		doom/i_net.c
# #		doom/info.c
# #		doom/i_sound.c
# #		doom/i_system.c
# #		doom/i_video.c
# #		doom/m_argv.c
# #		doom/m_bbox.c
# #		doom/m_cheat.c
# #		doom/m_fixed.c
# #		doom/m_menu.c
# #		doom/m_misc.c
# #		doom/m_random.c
# #		doom/m_swap.c
# #		doom/p_ceilng.c
# #		doom/p_doors.c
# #		doom/p_enemy.c
# #		doom/p_floor.c
# #		doom/p_inter.c
# #		doom/p_lights.c
# #		doom/p_map.c
# #		doom/p_maputl.c
# #		doom/p_mobj.c
# #		doom/p_plats.c
# #		doom/p_pspr.c
# #		doom/p_saveg.c
# #		doom/p_setup.c
# #		doom/p_sight.c
# #		doom/p_spec.c
# #		doom/p_switch.c
# #		doom/p_telept.c
# #		doom/p_tick.c
# #		doom/p_user.c
# #		doom/r_bsp.c
# #		doom/r_data.c
# #		doom/r_draw.c
# #		doom/r_main.c
# #		doom/r_plane.c
# #		doom/r_segs.c
# #		doom/r_sky.c
# #		doom/r_things.c
# #		doom/sounds.c
# #		doom/s_sound.c
# #		doom/st_lib.c
# #		doom/st_stuff.c
# #		doom/tables.c
# #		doom/v_video.c
# #		doom/wi_stuff.c
# #		doom/w_wad.c
# #		doom/z_zone.c)
# 
# target_include_directories(hitagicortex PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# target_include_directories(hitagisys PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# target_include_directories(hitagiclips PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# target_include_directories(hitagimon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# #set_target_properties(hitagimon PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/${LINKER_SCRIPT})
# target_link_libraries(hitagimon
# 		hitagiclips
# 		gcc
# 		c
# 		m
# 		hitagicortex
# 		hitagisys
# 		mon960
# 		stdc++
# 		c
# 		m
# 		gcc # necessary for soft float support, will need to provide my own custom routines ahead of this point to access the fpu
# 		mon960
# 		)