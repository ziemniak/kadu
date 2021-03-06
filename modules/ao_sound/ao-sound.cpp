/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <ao/ao.h>

#include "debug.h"

#include "../sound/sound.h"
#include "../sound/sound-params.h"

#include "ao-play-thread.h"

#include "ao-sound.h"


extern "C" int ao_sound_init()
{
	kdebugf();

	ao_player = new AOPlayer;
	if (!ao_player->isOk())
	{
		delete ao_player;
		return -1;
	}
	sound_manager->setPlayer(ao_player);

	kdebugf2();
	return 0;
}

extern "C" void ao_sound_close()
{
	kdebugf();
	delete ao_player;
	ao_player = 0;
	kdebugf2();
}

bool AOPlayer::isOk()
{
	return (thread);
}

AOPlayer::AOPlayer(QObject *parent) : SoundPlayer(parent)
{
	kdebugf();
	ao_initialize();
	
	thread = new AOPlayThread();
	if (!thread)
		return;
	thread->start();
	
	kdebugf2();
}

AOPlayer::~AOPlayer()
{
	kdebugf();

	if (thread)
	{
		thread->mutex.lock();
		thread->end = true;
		thread->mutex.unlock();
		thread->semaphore->release();
		thread->wait();
		delete thread;
		thread = 0;
	}

	ao_shutdown();
	kdebugf2();
}

void AOPlayer::playSound(const QString &path, bool volCntrl, double vol)
{
	kdebugf();

	if (thread->mutex.tryLock())
	{
		thread->list << SoundParams(path, volCntrl, vol);
		thread->mutex.unlock();
		thread->semaphore->release();
	}

	kdebugf2();
}

AOPlayer *ao_player;
