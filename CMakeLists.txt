cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(pachi LANGUAGES C)

find_package(Threads REQUIRED)

set(HEADER_FILES
	board.h
	gtp.h
	move.h
	ownermap.h
	pattern3.h
	pattern.h
	patternsp.h
	patternprob.h
	playout.h
	probdist.h
	random.h
	stone.h
	timeinfo.h
	network.h
	fbook.h
	chat.h

	mq.h
	gogui.h
	debug.h
	util.h
	engine.h
	version.h
#	dcnn.h
	stats.h
	fixp.h

	random/random.h

	replay/replay.h

	patternscan/patternscan.h

	patternplay/patternplay.h

	joseki/base.h
	joseki/joseki.c

	montecarlo/internal.h
	montecarlo/montecarlo.h

	uct/walk.h
	uct/policy/generic.h
	uct/tree.h
	uct/slave.h
	uct/search.h
	uct/plugins.h
	uct/plugin.h
	uct/dynkomi.h
	uct/uct.h
	uct/internal.h
	uct/prior.h

	playout/light.h
	playout/moggy.h

	tactics/2lib.h
	tactics/dragon.h
	tactics/nlib.h
	tactics/ladder.h
	tactics/util.h
	tactics/1lib.h
	tactics/seki.h
	tactics/selfatari.h
	tactics/nakade.h

	t-unit/test.h

	distributed/merge.h
	distributed/distributed.h
	distributed/protocol.h
)


set(SOURCE_FILES
	pachi.c
	ownermap.c
	pattern.c
	playout.c
	network.c
	board.c
	chat.c
	patternprob.c
	patternsp.c
	stone.c
	probdist.c
	move.c
	random.c
	timeinfo.c
	fbook.c
	pattern3.c
	gtp.c

	playout/moggy.c
	playout/light.c

	patternscan/patternscan.c
	
	distributed/merge.c
	distributed/protocol.c
	distributed/distributed.c
	
	t-unit/test.c
	t-unit/test_undo.c
	
	uct/dynkomi.c
	uct/policy/ucb1amaf.c
	uct/policy/ucb1.c
	uct/policy/generic.c
	uct/search.c
	uct/plugins.c
	uct/prior.c
	uct/slave.c
	uct/uct.c
	uct/tree.c
	uct/walk.c
	
	random/random.c

	replay/replay.c

	joseki/joseki.c
	joseki/base.c

	tactics/2lib.c
	tactics/util.c
	tactics/ladder.c
	tactics/nakade.c
	tactics/nlib.c
	tactics/1lib.c
	tactics/seki.c
	tactics/selfatari.c
	tactics/dragon.c

	patternplay/patternplay.c

	montecarlo/montecarlo.c
)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

if(NOT MSVC)
    target_link_libraries(${PROJECT_NAME} PUBLIC m)
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})	