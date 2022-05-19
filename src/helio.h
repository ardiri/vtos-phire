/*
 * @(#)helio.h
 *
 * Copyright 1999-2001, Aaron Ardiri (mailto:aaron@ardiri.com)
 * All rights reserved.
 *
 * This  file was generated as  part of the "phire!" program  developed 
 * for the Helio Computing Platform designed by vtech:
 *
 *   http://www.vtechinfo.com/
 *
 * The contents of this file is confidential and  proprietary in nature
 * ("Confidential Information"). Redistribution or modification without 
 * prior consent of the original author is prohibited.
 */

#ifndef _HELIO_H
#define _HELIO_H

// system includes
#include <system.h>
#include "resource/gfx.h"

// application constants
#define GAME_FPS  8                  // 8 fps (adequate for animations)
#define MAX_JUMP  22                 // 21 jumpers, one starter

typedef struct
{
  struct 
  {
    BOOLEAN   gamePlaying;              // is there a game in play?
    BOOLEAN   gamePaused;               // is the game currently paused?
    BOOLEAN   gameWait;                 // are we waiting for user? 
    USHORT    gameAnimationCount;       // a ticking counter
    USHORT    gameScore;                // the score of the player
    USHORT    highScore[2];             // a high score list (score only)
    USHORT    gameLives;                // the number of lives the player has

    struct 
    {
      BYTE    gameType;                 // what type of game are we playing?
      USHORT  gameLevel;                // what level are we at?
      BOOLEAN bonusAvailable;           // is there a bonus available?
      BOOLEAN bonusScoring;             // are we currently in BONUS mode?

      USHORT  smokeWait;                // the delay between smoke positions
      USHORT  smokePosition;            // the position of the smoke

      USHORT  trampolinePosition;       // the position of the trampoline
      USHORT  trampolineNewPosition;    // the desired position of trampoline

      USHORT  jumperCount;              // the number of jumpers on screen
      USHORT  jumperWait[MAX_JUMP];     // the delay between jumper moves
      USHORT  jumperPosition[MAX_JUMP]; // the position of the jumper

      USHORT  jumperDeathPosition;      // the index of death?
    } phire;

  } game;
  
} PreferencesType;

// local includes
#include "device.h"
#include "help.h"
#include "game.h"
#include "animate.h"
#include "resource.h"

// functions
extern void    InitApplication(void);
extern BOOLEAN ApplicationHandleEvent(EvtType *);
extern void    ApplicationDisplayDialog(ObjectID);
extern void    EventLoop(void);
extern void    EndApplication(void);
extern BOOLEAN HelioMain(WORD, void *);

#endif 
