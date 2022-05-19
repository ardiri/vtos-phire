/*
 * @(#)game.c
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

#include "helio.h"

// image resources
#include "game.inc"

// interface
static void GameGetSpritePosition(BYTE, BYTE, SHORT *, SHORT *);
static void GameAdjustLevel(PreferencesType *);
static void GameIncrementScore(PreferencesType *);
static void GameMovePlayer(PreferencesType *);
static void GameMoveJumpers(PreferencesType *);
static void GameRemoveJumper(PreferencesType *, SHORT);

// global variable structure
typedef struct
{
  GfxWindow *winDigits;                     // scoring digits bitmaps
  GfxWindow *winMisses;                     // the lives notification bitmaps

  GfxWindow *winSmokes;                     // the smoke bitmaps
  BOOLEAN   smokeChanged;                   // do we need to repaint the smoke?
  BOOLEAN   smokeOnScreen;                  // is smoke bitmap on screen?
  USHORT    smokeOldPosition;               // the *old* position of the smoke 

  GfxWindow *winTrampolines;                // the trampoline bitmaps
  BOOLEAN   trampolineChanged;              // do we need to repaint trampoline?
  BOOLEAN   trampolineOnScreen;             // is trampoline bitmap on screen?
  USHORT    trampolineOldPosition;          // the *old* position of trampoline 

  GfxWindow *winJumpers;                     // the jumper bitmaps
  BOOLEAN   jumperChanged[MAX_JUMP];         // do we need to repaint parachute
  BOOLEAN   jumperOnScreen[MAX_JUMP];        // is parachute bitmap on screen?
  USHORT    jumperOnScreenPosition[MAX_JUMP];// the *old* position of parachute 

  GfxWindow *winJumperDeaths;               // the jumper death bitmaps
  
  BYTE      gameType;                       // the type of game active
  BOOLEAN   playerDied;                     // has the player died?
  BYTE      moveDelayCount;                 // the delay between moves
  BYTE      moveLast;                       // the last move performed
  BYTE      moveNext;                       // the next desired move  
} GameGlobals;

// globals reference
static GameGlobals *gbls;

/**
 * Initialize the Game.
 */  
void   
GameInitialize()
{
  // create the globals object
  gbls = (GameGlobals *)pmalloc(sizeof(GameGlobals));

  // initialize and load the "bitmap" windows
  {
    SHORT i;

    gbls->winDigits                = (GfxWindow *)pmalloc(sizeof(GfxWindow));
    gbls->winDigits->width         = bitmap00Width;
    gbls->winDigits->height        = bitmap00Height;
    gbls->winDigits->memSize       = bitmap00Size;
    gbls->winDigits->memory        = (void *)bitmap00;
    gbls->winMisses                = (GfxWindow *)pmalloc(sizeof(GfxWindow));
    gbls->winMisses->width         = bitmap01Width;
    gbls->winMisses->height        = bitmap01Height;
    gbls->winMisses->memSize       = bitmap01Size;
    gbls->winMisses->memory        = (void *)bitmap01;
    gbls->winSmokes                = (GfxWindow *)pmalloc(sizeof(GfxWindow));
    gbls->winSmokes->width         = bitmap02Width;
    gbls->winSmokes->height        = bitmap02Height;
    gbls->winSmokes->memSize       = bitmap02Size;
    gbls->winSmokes->memory        = (void *)bitmap02;
    gbls->winTrampolines           = (GfxWindow *)pmalloc(sizeof(GfxWindow));
    gbls->winTrampolines->width    = bitmap03Width;
    gbls->winTrampolines->height   = bitmap03Height;
    gbls->winTrampolines->memSize  = bitmap03Size;
    gbls->winTrampolines->memory   = (void *)bitmap03;
    gbls->winJumpers               = (GfxWindow *)pmalloc(sizeof(GfxWindow));
    gbls->winJumpers->width        = bitmap04Width;
    gbls->winJumpers->height       = bitmap04Height;
    gbls->winJumpers->memSize      = bitmap04Size;
    gbls->winJumpers->memory       = (void *)bitmap04;
    gbls->winJumperDeaths          = (GfxWindow *)pmalloc(sizeof(GfxWindow));
    gbls->winJumperDeaths->width   = bitmap05Width;
    gbls->winJumperDeaths->height  = bitmap05Height;
    gbls->winJumperDeaths->memSize = bitmap05Size;
    gbls->winJumperDeaths->memory  = (void *)bitmap05;

    gbls->smokeChanged             = TRUE;
    gbls->smokeOnScreen            = FALSE;
    gbls->smokeOldPosition         = 0;
    
    gbls->trampolineChanged        = TRUE;
    gbls->trampolineOnScreen       = FALSE;
    gbls->trampolineOldPosition    = 0;

    for (i=0; i<MAX_JUMP; i++) {
      gbls->jumperChanged[i]          = TRUE;
      gbls->jumperOnScreen[i]         = FALSE;
      gbls->jumperOnScreenPosition[i] = 0;
    }
  }
}

/**
 * Reset the Game.
 * 
 * @param prefs the global preference data.
 * @param gameType the type of game to configure for.
 */  
void   
GameReset(PreferencesType *prefs, BYTE gameType)
{
  // turn off all the "bitmaps"
  FormDrawForm(gameForm);

  // turn on all the "bitmaps"
  {
    GfxRegion region    = { {   0,   0 }, {   0,   0 } };
    GfxRegion scrRegion = { {   0,   0 }, {   0,   0 } };
    SHORT     i;

    //
    // draw the score
    //

    for (i=0; i<4; i++) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteDigit, i,
                            &scrRegion.topLeft.x, &scrRegion.topLeft.y);
      scrRegion.extent.x  = 7;
      scrRegion.extent.y  = 12;
      region.topLeft.x    = 8 * scrRegion.extent.x;
      region.topLeft.y    = 0;
      region.extent.x     = scrRegion.extent.x;
      region.extent.y     = scrRegion.extent.y;

      // draw the digit!
      GfxCopyRegion(gbls->winDigits, GfxGetDrawWindow(),
                    &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxPaint);
    }

    //
    // draw the misses
    //

    // what is the rectangle we need to copy?
    GameGetSpritePosition(spriteMiss, 0,
                          &scrRegion.topLeft.x, &scrRegion.topLeft.y);
    scrRegion.extent.x  = 30;
    scrRegion.extent.y  = 20;
    region.topLeft.x    = 2 * scrRegion.extent.x;
    region.topLeft.y    = 0;
    region.extent.x     = scrRegion.extent.x;
    region.extent.y     = scrRegion.extent.y;

    // draw the miss bitmap!
    GfxCopyRegion(gbls->winMisses, GfxGetDrawWindow(),
                  &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxOverlay);

    //
    // draw the smoke
    //

    for (i=0; i<4; i++) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteSmoke, 0, 
                            &scrRegion.topLeft.x, &scrRegion.topLeft.y);
      scrRegion.extent.x  = 34;
      scrRegion.extent.y  = 21;
      region.topLeft.x    = i * scrRegion.extent.x; 
      region.topLeft.y    = 0;  
      region.extent.x     = scrRegion.extent.x;
      region.extent.y     = scrRegion.extent.y;

      // draw the smoke bitmap!
      GfxCopyRegion(gbls->winSmokes, GfxGetDrawWindow(),
                    &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxOverlay);
    }
    
    //
    // draw the jumpers
    //

    for (i=0; i<2; i++) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteJumper, i,
                            &scrRegion.topLeft.x, &scrRegion.topLeft.y);
      scrRegion.extent.x  = 14;
      scrRegion.extent.y  = 14;
      region.topLeft.x    = i * scrRegion.extent.x;
      region.topLeft.y    = 0;
      region.extent.x     = scrRegion.extent.x;
      region.extent.y     = scrRegion.extent.y;

      // draw the jumper bitmap!
      GfxCopyRegion(gbls->winJumpers, GfxGetDrawWindow(),
                    &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxOverlay);
    }

    for (i=7; i<28; i++) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteJumper, i,
                            &scrRegion.topLeft.x, &scrRegion.topLeft.y);
      scrRegion.extent.x  = 14;
      scrRegion.extent.y  = 14;
      region.topLeft.x    = (i % 7) * scrRegion.extent.x;
      region.topLeft.y    = (i / 7) * scrRegion.extent.y;
      region.extent.x     = scrRegion.extent.x;
      region.extent.y     = scrRegion.extent.y;

      // draw the jumper bitmap!
      GfxCopyRegion(gbls->winJumpers, GfxGetDrawWindow(),
                    &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxOverlay);
    }

    //
    // draw the jumper deaths :))
    //

    for (i=0; i<3; i++) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteJumperDeath, i, 
                            &scrRegion.topLeft.x, &scrRegion.topLeft.y);
      scrRegion.extent.x  = 21;
      scrRegion.extent.y  = 10;
      region.topLeft.x    = i * scrRegion.extent.x; 
      region.topLeft.y    = 0;
      region.extent.x     = scrRegion.extent.x;
      region.extent.y     = scrRegion.extent.y;

      // draw the jumper bitmap!
      GfxCopyRegion(gbls->winJumperDeaths, GfxGetDrawWindow(),
                    &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxOverlay);
    }

    // 
    // draw the trampoline
    //

    for (i=0; i<3; i++) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteTrampoline, i, 
                            &scrRegion.topLeft.x, &scrRegion.topLeft.y);
      scrRegion.extent.x  = 35;
      scrRegion.extent.y  = 20;
      region.topLeft.x    = i * scrRegion.extent.x; 
      region.topLeft.y    = 0;
      region.extent.x     = scrRegion.extent.x;
      region.extent.y     = scrRegion.extent.y;

      // draw the boat bitmap
      GfxCopyRegion(gbls->winTrampolines, GfxGetDrawWindow(),
                    &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxOverlay);
    }
  }

  // wait a good two seconds :))
  TmrWaitTime(2000);

  // turn off all the "bitmaps"
  FormDrawForm(gameForm);

  // reset the preferences
  GameResetPreferences(prefs, gameType);}

/**
 * Reset the Game preferences.
 * 
 * @param prefs the global preference data.
 * @param gameType the type of game to configure for.
 */  
void   
GameResetPreferences(PreferencesType *prefs, BYTE gameType)
{
  SHORT i;

  // now we are playing
  prefs->game.gamePlaying                   = TRUE;
  prefs->game.gamePaused                    = FALSE;
  prefs->game.gameWait                      = TRUE;
  prefs->game.gameAnimationCount            = 0;

  // reset score and lives
  prefs->game.gameScore                     = 0;
  prefs->game.gameLives                     = 3;

  // reset phire specific things
  prefs->game.phire.gameType                = gameType;
  prefs->game.phire.gameLevel               = 1;
  prefs->game.phire.bonusAvailable          = TRUE;
  prefs->game.phire.bonusScoring            = FALSE;

  prefs->game.phire.smokeWait               = 0;
  prefs->game.phire.smokePosition           = 0;

  prefs->game.phire.trampolinePosition      = 0;
  prefs->game.phire.trampolineNewPosition   = 0;

  prefs->game.phire.jumperCount             = 0;
  memset(prefs->game.phire.jumperPosition, (UBYTE)0, sizeof(USHORT) * MAX_JUMP);
  memset(prefs->game.phire.jumperWait, (UBYTE)0, sizeof(USHORT) * MAX_JUMP);
  prefs->game.phire.jumperDeathPosition     = 0;

  // reset the "backup" and "onscreen" flags
  gbls->smokeChanged                        = TRUE;
  gbls->trampolineChanged                   = TRUE;
  for (i=0; i<MAX_JUMP; i++) {
    gbls->jumperChanged[i]                  = TRUE;
    gbls->jumperOnScreen[i]                 = FALSE;
  }

  gbls->gameType                            = gameType;
  gbls->playerDied                          = FALSE;
  gbls->moveDelayCount                      = 0;
  gbls->moveLast                            = moveNone;
  gbls->moveNext                            = moveNone;
}

/**
 * Process key input from the user.
 * 
 * @param prefs the global preference data.
 * @param keyStatus the current key state.
 */  
void   
GameProcessKeyInput(PreferencesType *prefs, UWORD keyStatus)
{
  // the helio device does not have a very nice "key" pattern so
  // playing games using the keys may not be an easy task :) the
  // following is coded as a "prototype", maybe someone will use
  // the "key" capabilities. :))
  //
  // the system is hardcoded as follows:
  //
  //   address book | pageUp    = move left
  //   to do list   | pageDown  = move right
  //
  // :P enjoy
  //
  // -- Aaron Ardiri, 2000

#define ctlKeyLeft  (keyBitHard1 | keyBitPageUp)
#define ctlKeyRight (keyBitHard2 | keyBitPageDown)

  keyStatus &= (ctlKeyLeft  |
                ctlKeyRight);

  // did they press at least one of the game keys?
  if (keyStatus != 0) {

    // if they were waiting, we should reset the game animation count
    if (prefs->game.gameWait) { 
      prefs->game.gameAnimationCount = 0;
      prefs->game.gameWait           = FALSE;
    }

    // great! they wanna play
    prefs->game.gamePaused = FALSE;
  }

  // move left
  if (
      ((keyStatus & ctlKeyLeft) != 0) &&
      (
       (gbls->moveDelayCount == 0) || 
       (gbls->moveLast       != moveLeft)
      )
     ) {

    // adjust the position if possible
    if (prefs->game.phire.trampolinePosition > 0) {
      prefs->game.phire.trampolineNewPosition = 
        prefs->game.phire.trampolinePosition - 1;
    }
  }

  // move right
  else
  if (
      ((keyStatus & ctlKeyRight) != 0) &&
      (
       (gbls->moveDelayCount == 0) || 
       (gbls->moveLast       != moveRight)
      )
     ) {

    // adjust the position if possible
    if (prefs->game.phire.trampolinePosition < 2) {
      prefs->game.phire.trampolineNewPosition = 
        prefs->game.phire.trampolinePosition + 1;
    }
  }
}

/**
 * Process stylus input from the user.
 * 
 * @param prefs the global preference data.
 * @param x the x co-ordinate of the stylus event.
 * @param y the y co-ordinate of the stylus event.
 */  
void   
GameProcessStylusInput(PreferencesType *prefs, SHORT x, SHORT y)
{
  GfxRegion region;
  SHORT     i;

  // lets take a look at all the possible "positions"
  for (i=0; i<3; i++) {

    // get the bounding box of the position
    GameGetSpritePosition(spriteTrampoline, i,
                          &region.topLeft.x, &region.topLeft.y);
    region.extent.x  = 35;
    region.extent.y  = 20;

    // did they tap inside this rectangle?
    if (((x >= region.topLeft.x) && (y >= region.topLeft.y) && 
	 (x <= (region.topLeft.x+region.extent.x)) &&
	 (y <= (region.topLeft.y+region.extent.y)))) {

      // ok, this is where we are going to go :)
      prefs->game.phire.trampolineNewPosition = i;

      // if they were waiting, we should reset the game animation count
      if (prefs->game.gameWait) { 
        prefs->game.gameAnimationCount = 0;
        prefs->game.gameWait           = FALSE;
      }

      // great! they wanna play
      prefs->game.gamePaused = FALSE;
      break;                                        // stop looking
    }
  }	
}

/**
 * Process the object movement in the game.
 * 
 * @param prefs the global preference data.
 */  
void   
GameMovement(PreferencesType *prefs)
{
  SHORT     i, j;
  GfxRegion region  = {{   8,  18 }, { 144, 14 }};

  //
  // the game is NOT paused.
  //

  if (!prefs->game.gamePaused) {

    // animate the smoke
    if (prefs->game.phire.smokeWait == 0) {
    
      prefs->game.phire.smokePosition =
        (prefs->game.phire.smokePosition + 1) % 4;
      prefs->game.phire.smokeWait     = 4;
  
      gbls->smokeChanged = TRUE;
    }
    else {
      prefs->game.phire.smokeWait--;
    }

    // we must make sure the user is ready for playing 
    if (!prefs->game.gameWait) {

      // we cannot be dead yet :)
      gbls->playerDied = FALSE;

      // are we in bonus mode?
      if ((prefs->game.phire.bonusScoring) &&
          (prefs->game.gameAnimationCount % GAME_FPS) < (GAME_FPS >> 1)) {

        BYTE    str[32];
        GfxFont currFont = GfxGetFont();

        strcpy(str, "    * BONUS PLAY *    ");
        GfxSetFont(gfx_palmosBoldFont);
        GfxDrawString(str, strlen(str), 
                      80 - (GfxGetCharsWidth(str, strlen(str)) >> 1), 22, gfxPaint);
        GfxSetFont(currFont);
      }
      else 
        GfxFillRegion(GfxGetDrawWindow(), &region, gfx_white);

      // player gets first move
      GameMovePlayer(prefs);
      GameMoveJumpers(prefs);

      // is it time to upgrade the game?
      if (prefs->game.gameAnimationCount >= 
           ((gbls->gameType == GAME_A) ? 0x17f : 0x100)) {

        prefs->game.gameAnimationCount = 0;
        prefs->game.phire.gameLevel++;

        // upgrading of difficulty?
        if (
            (gbls->gameType              == GAME_A) &&
            (prefs->game.phire.gameLevel > 12)
           ) {

          gbls->gameType               = GAME_B;
          prefs->game.phire.gameLevel -= 2; // give em a break :)
        }
      } 

      // has the player died in this frame?
      if (gbls->playerDied) {

        SHORT     i, index;
        GfxRegion region    = { {   0,   0 }, {   0,   0 } };
        GfxRegion scrRegion = { {   0,   0 }, {   0,   0 } };

        // play death sound and flash the player
        for (i=0; i<4; i++) {

          index = prefs->game.phire.jumperDeathPosition;

          // what is the rectangle we need to copy?
          GameGetSpritePosition(spriteJumperDeath, index, 
                                &scrRegion.topLeft.x, &scrRegion.topLeft.y);
          scrRegion.extent.x  = 21;
          scrRegion.extent.y  = 10;
          region.topLeft.x    = index * scrRegion.extent.x; 
          region.topLeft.y    = 0;
          region.extent.x     = scrRegion.extent.x;
          region.extent.y     = scrRegion.extent.y;

          // invert the jumper death bitmap!
          GfxCopyRegion(gbls->winJumperDeaths, GfxGetDrawWindow(),
                        &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxInvert);

          // play death sound
          SndPlaySndEffect(SNDRES2_BEEP);
          TmrWaitTime(500);
        }

        // lose a life :(
        prefs->game.gameLives--;

        // no more lives left: GAME OVER!
        if (prefs->game.gameLives == 0) {

          // return to main screen
          EvtAppendEvt(EVT_INLAY_SELECT,0,INLAY_EXIT,0,NULL);

          prefs->game.gamePlaying = FALSE;
        }

        // reset player position and continue game
        else {
          GameAdjustLevel(prefs);
          prefs->game.phire.bonusScoring = FALSE;
          prefs->game.gameWait           = TRUE;
        }
      }
    }

    // we have to display "GET READY!"
    else {

      // flash on:
      if ((prefs->game.gameAnimationCount % GAME_FPS) < (GAME_FPS >> 1)) {

        BYTE    str[32];
        GfxFont currFont = GfxGetFont();

        strcpy(str, "    * GET READY *    ");
        GfxSetFont(gfx_palmosBoldFont);
        GfxDrawString(str, strlen(str), 
                      80 - (GfxGetCharsWidth(str, strlen(str)) >> 1), 22, gfxPaint);
        GfxSetFont(currFont);
      }

      // flash off:
      else
        GfxFillRegion(GfxGetDrawWindow(), &region, gfx_white);
    }

    // update the animation counter
    prefs->game.gameAnimationCount++;
  }

  //
  // the game is paused.
  //

  else {

    BYTE    str[32];
    GfxFont currFont = GfxGetFont();

    strcpy(str, "    *  PAUSED  *    ");
    GfxSetFont(gfx_palmosBoldFont);
    GfxDrawString(str, strlen(str), 
                  80 - (GfxGetCharsWidth(str, strlen(str)) >> 1), 22, gfxPaint);
    GfxSetFont(currFont);
  }
}

/**
 * Draw the game on the screen.
 * 
 * @param prefs the global preference data.
 */
void   
GameDraw(PreferencesType *prefs)
{
  SHORT     i, index;
  GfxRegion region    = { {   0,   0 }, {   0,   0 } };
  GfxRegion scrRegion = { {   0,   0 }, {   0,   0 } };

  // 
  // DRAW INFORMATION/BITMAPS ON SCREEN
  //

  // draw the score
  {
    short base;
 
    base = 1000;  // max score (4 digits)
    for (i=0; i<4; i++) {

      index = (prefs->game.gameScore / base) % 10;

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteDigit, i, 
                            &scrRegion.topLeft.x, &scrRegion.topLeft.y);
      scrRegion.extent.x  = 7;
      scrRegion.extent.y  = 12;
      region.topLeft.x    = index * scrRegion.extent.x;
      region.topLeft.y    = 0;
      region.extent.x     = scrRegion.extent.x;
      region.extent.y     = scrRegion.extent.y;

      // draw the digit!
      GfxCopyRegion(gbls->winDigits, GfxGetDrawWindow(),
                    &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxPaint);
      base /= 10;
    }
  }

  // draw the misses that have occurred :( 
  if (prefs->game.gameLives < 3) {
  
    index = 2 - prefs->game.gameLives;

    // what is the rectangle we need to copy?
    GameGetSpritePosition(spriteMiss, 0, 
                          &scrRegion.topLeft.x, &scrRegion.topLeft.y);
    scrRegion.extent.x  = 30;
    scrRegion.extent.y  = 20;
    region.topLeft.x    = index * scrRegion.extent.x;
    region.topLeft.y    = 0;
    region.extent.x     = scrRegion.extent.x;
    region.extent.y     = scrRegion.extent.y;

    // draw the miss bitmap!
    GfxCopyRegion(gbls->winMisses, GfxGetDrawWindow(),
                  &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxOverlay);
  }
  // no missed, make sure none are shown
  else {
  
    index = 2;  // the miss with *all* three misses

    // what is the rectangle we need to copy?
    GameGetSpritePosition(spriteMiss, 0, 
                          &scrRegion.topLeft.x, &scrRegion.topLeft.y);
    scrRegion.extent.x  = 30;
    scrRegion.extent.y  = 20;
    region.topLeft.x    = index * scrRegion.extent.x;
    region.topLeft.y    = 0;
    region.extent.x     = scrRegion.extent.x;
    region.extent.y     = scrRegion.extent.y;

    // invert the three miss bitmap!
    GfxCopyRegion(gbls->winMisses, GfxGetDrawWindow(),
                  &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxMask);
  }

  // draw the smoke on the screen (only if it has changed)
  if (gbls->smokeChanged) {

    // what is the rectangle we need to copy?
    GameGetSpritePosition(spriteSmoke, 0, 
                          &scrRegion.topLeft.x, &scrRegion.topLeft.y);
    scrRegion.extent.x  = 34;
    scrRegion.extent.y  = 21;
    region.topLeft.x    = 0;  
    region.topLeft.y    = 0;  
    region.extent.x     = scrRegion.extent.x;
    region.extent.y     = scrRegion.extent.y;

    // 
    // erase the previous smoke 
    // 

    if (gbls->smokeOnScreen) {

      index = gbls->smokeOldPosition;

      // what is the rectangle we need to copy?
      region.topLeft.x = index * scrRegion.extent.x; 
      region.topLeft.y = 0;
      region.extent.x  = scrRegion.extent.x;
      region.extent.y  = scrRegion.extent.y;

      // invert the smoke bitmap!
      GfxCopyRegion(gbls->winSmokes, GfxGetDrawWindow(),
                    &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxMask);
      gbls->smokeOnScreen = FALSE;
    }

    // 
    // draw the smoke at the new position
    // 

    index = prefs->game.phire.smokePosition;

    // what is the rectangle we need to copy?
    region.topLeft.x = index * scrRegion.extent.x; 
    region.topLeft.y = 0;
    region.extent.x  = scrRegion.extent.x;
    region.extent.y  = scrRegion.extent.y;

    // save this location, record blade is onscreen
    gbls->smokeOnScreen    = TRUE;
    gbls->smokeOldPosition = index;

    // draw the smoke bitmap!
    GfxCopyRegion(gbls->winSmokes, GfxGetDrawWindow(),
                  &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxOverlay);

    // dont draw until we need to
    gbls->smokeChanged = FALSE;
  }

  // draw the jumpers
  for (i=0; i<prefs->game.phire.jumperCount; i++) {

    // draw the jumper on the screen (only if it has changed)
    if (gbls->jumperChanged[i]) {

      //
      // erase the previous parachuter
      //
 
      if (gbls->jumperOnScreen[i]) {

        index = gbls->jumperOnScreenPosition[i];

        // what is the rectangle we need to copy?
        GameGetSpritePosition(spriteJumper, index,
                              &scrRegion.topLeft.x, &scrRegion.topLeft.y);
        scrRegion.extent.x  = 14;
        scrRegion.extent.y  = 14;
        region.topLeft.x    = (index % 7) * scrRegion.extent.x;
        region.topLeft.y    = (index / 7) * scrRegion.extent.y;
        region.extent.x     = scrRegion.extent.x;
        region.extent.y     = scrRegion.extent.y;

        // invert the old jumper bitmap!
        GfxCopyRegion(gbls->winJumpers, GfxGetDrawWindow(),
                      &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxMask);
      }

      //
      // draw the jumper at the new position
      //

      index = prefs->game.phire.jumperPosition[i];

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteJumper, index,
                            &scrRegion.topLeft.x, &scrRegion.topLeft.y);
      scrRegion.extent.x  = 14;
      scrRegion.extent.y  = 14;
      region.topLeft.x    = (index % 7) * scrRegion.extent.x;
      region.topLeft.y    = (index / 7) * scrRegion.extent.y;
      region.extent.x     = scrRegion.extent.x;
      region.extent.y     = scrRegion.extent.y;

      // save this location, record jumper is onscreen
      gbls->jumperOnScreen[i]         = TRUE;
      gbls->jumperOnScreenPosition[i] = prefs->game.phire.jumperPosition[i];

      // draw the jumper bitmap!
      GfxCopyRegion(gbls->winJumpers, GfxGetDrawWindow(),
                    &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxOverlay);

      // dont draw until we need to
      gbls->jumperChanged[i] = FALSE;
    }
  }

  // draw trampoline (only if it has changed)
  if (gbls->trampolineChanged) {

    // 
    // erase the previous trampoline
    // 

    if (gbls->trampolineOnScreen) {

      index = gbls->trampolineOldPosition;

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteTrampoline, index, 
                            &scrRegion.topLeft.x, &scrRegion.topLeft.y);
      scrRegion.extent.x  = 35;
      scrRegion.extent.y  = 20;
      region.topLeft.x    = index * scrRegion.extent.x; 
      region.topLeft.y    = 0;
      region.extent.x     = scrRegion.extent.x;
      region.extent.y     = scrRegion.extent.y;

      // invert the old trampoline bitmap
      GfxCopyRegion(gbls->winTrampolines, GfxGetDrawWindow(),
                    &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxMask);
      gbls->trampolineOnScreen  = FALSE;
    }

    // 
    // draw trampoline at the new position
    // 

    index = prefs->game.phire.trampolinePosition;

    // what is the rectangle we need to copy?
    GameGetSpritePosition(spriteTrampoline, index, 
                          &scrRegion.topLeft.x, &scrRegion.topLeft.y);
    scrRegion.extent.x  = 35;
    scrRegion.extent.y  = 20;
    region.topLeft.x    = index * scrRegion.extent.x; 
    region.topLeft.y    = 0;
    region.extent.x     = scrRegion.extent.x;
    region.extent.y     = scrRegion.extent.y;

    // save this location, record boat is onscreen
    gbls->trampolineOnScreen    = TRUE;
    gbls->trampolineOldPosition = index;

    // draw the trampoline bitmap!
    GfxCopyRegion(gbls->winTrampolines, GfxGetDrawWindow(),
                  &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxOverlay);

    // dont draw until we need to
    gbls->trampolineChanged = FALSE;
  }
}

/**
 * Terminate the game.
 */
void   
GameTerminate()
{
  // clean up windows/memory
  pfree(gbls->winDigits);
  pfree(gbls->winMisses);
  pfree(gbls->winSmokes);
  pfree(gbls->winTrampolines);
  pfree(gbls->winJumpers);
  pfree(gbls->winJumperDeaths);
  pfree(gbls);
}

/**
 * Get the position of a particular sprite on the screen.
 *
 * @param spriteType the type of sprite.
 * @param index the index required in the sprite position list.
 * @param x the x co-ordinate of the position
 * @param y the y co-ordinate of the position
 */
static void
GameGetSpritePosition(BYTE  spriteType, 
                      BYTE  index, 
                      SHORT *x, 
                      SHORT *y)
{
  switch (spriteType) 
  {
    case spriteDigit: 
         {
           *x = 69 + (index * 9);
           *y = 38;
         }
         break;

    case spriteMiss: 
         {
           *x = 126;
           *y = 37;
         }
         break;

    case spriteSmoke: 
         {
           *x = 13;
           *y = 36;
         }
         break;

    case spriteTrampoline: 
         {
           SHORT positions[][2] = {
                                   {  13, 104 },
                                   {  54, 103 },
                                   {  93, 105 }
                                 };

           *x = positions[index][0];
           *y = positions[index][1];
         }
         break;

    case spriteJumper: 
         {
           SHORT positions[][2] = {
                                   {   9,  57 }, // jump starters
                                   {   9,  74 },
                                   {   0,   0 },
                                   {   0,   0 },
                                   {   0,   0 },
                                   {   0,   0 },
                                   {   0,   0 },
                                   {  18,  64 }, // falling
                                   {  18,  79 },
                                   {  19,  92 },
                                   {  25, 103 },
                                   {  30,  91 },
                                   {  36,  79 },
                                   {  39,  65 },
                                   {  48,  56 },
                                   {  57,  66 },
                                   {  58,  80 },
                                   {  60,  93 },
                                   {  65, 104 },
                                   {  72,  90 },
                                   {  79,  79 },
                                   {  87,  70 },
                                   {  93,  80 },
                                   {  98,  92 },
                                   { 104, 104 },
                                   { 111,  90 },
                                   { 116,  82 },
                                   { 129,  88 }
                                  };

           *x = positions[index][0];
           *y = positions[index][1];
         }
         break;

    case spriteJumperDeath: 
         {
           SHORT positions[][2] = {
                                   {  24, 127 },
                                   {  62, 126 },
                                   { 102, 126 },
                                   {  43, 127 },
                                   {  71, 127 },
                                   { 118, 123 }
                                 };

           *x = positions[index][0];
           *y = positions[index][1];
         }
         break;

    default:
         break;
  }
}

/**
 * Adjust the level (reset positions)
 *
 * @param prefs the global preference data.
 */
static void 
GameAdjustLevel(PreferencesType *prefs)
{
  // player should stay were the were
  prefs->game.phire.trampolineNewPosition = prefs->game.phire.trampolinePosition;
  gbls->trampolineChanged                 = TRUE;

  // player is not dead
  gbls->playerDied                        = FALSE;
}

/**
 * Increment the players score. 
 *
 * @param prefs the global preference data.
 */
static void 
GameIncrementScore(PreferencesType *prefs)
{
  SHORT      i, index;
  GfxRegion  region     = { {   0,   0 }, {   0,   0 } };
  GfxRegion  scrRegion  = { {   0,   0 }, {   0,   0 } };

  // adjust accordingly
  prefs->game.gameScore += prefs->game.phire.bonusScoring ? 2 : 1;

  // redraw score bitmap
  {
    SHORT base;
 
    base = 1000;  // max score (4 digits)
    for (i=0; i<4; i++) {

      index = (prefs->game.gameScore / base) % 10;

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteDigit, i, 
                            &scrRegion.topLeft.x, &scrRegion.topLeft.y);
      scrRegion.extent.x  = 7;
      scrRegion.extent.y  = 12;
      region.topLeft.x    = index * scrRegion.extent.x;
      region.topLeft.y    = 0;
      region.extent.x     = scrRegion.extent.x;
      region.extent.y     = scrRegion.extent.y;

      // draw the digit!
      GfxCopyRegion(gbls->winDigits, GfxGetDrawWindow(),
                    &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxPaint);
      base /= 10;
    }
  }

  // play the sound
  SndPlaySndEffect(SNDRES5_BEEP);
  TmrWaitTime(125);

  // is it time for a bonus?
  if (
      (prefs->game.gameScore >= 300) &&
      (prefs->game.phire.bonusAvailable)
     ) {

    // little fan-fare :)) - the "veewoo" sound was the best i could find :((
    SndPlaySndEffect(SNDRES_VEEWOO);
    TmrWaitTime(500);

    // apply the bonus!
    if (prefs->game.gameLives == 3) 
      prefs->game.phire.bonusScoring = TRUE;
    else
      prefs->game.gameLives = 3;

    prefs->game.phire.bonusAvailable = FALSE;
  }
}

/**
 * Move the player.
 *
 * @param prefs the global preference data.
 */
static void
GameMovePlayer(PreferencesType *prefs) 
{
  //
  // where does boat want to go today?
  //

  // current position differs from new position?
  if (prefs->game.phire.trampolinePosition != 
      prefs->game.phire.trampolineNewPosition) {

    // need to move left
    if (prefs->game.phire.trampolinePosition > 
        prefs->game.phire.trampolineNewPosition) {

      gbls->moveNext = moveLeft;
    }

    // need to move right
    else
    if (prefs->game.phire.trampolinePosition < 
        prefs->game.phire.trampolineNewPosition) {

      gbls->moveNext = moveRight;
    }
  }

  // lets make sure they are allowed to do the move
  if (
      (gbls->moveDelayCount == 0) || 
      (gbls->moveLast != gbls->moveNext) 
     ) {
    gbls->moveDelayCount = 
     ((gbls->gameType == GAME_A) ? 4 : 3);
  }
  else {
    gbls->moveDelayCount--;
    gbls->moveNext = moveNone;
  }

  // which direction do they wish to move?
  switch (gbls->moveNext)
  {
    case moveLeft:
         {
           prefs->game.phire.trampolinePosition--;
           gbls->trampolineChanged = TRUE;
         }
         break;

    case moveRight:
         {
           prefs->game.phire.trampolinePosition++;
           gbls->trampolineChanged = TRUE;
         }
         break;

    default:
         break;
  }

  gbls->moveLast = gbls->moveNext;
  gbls->moveNext = moveNone;

  // do we need to play a movement sound? 
  if (gbls->trampolineChanged)  
    SndPlaySndEffect(SNDRES0_BEEP);
}

/**
 * Move the jumpers.
 *
 * @param prefs the global preference data.
 */
static void
GameMoveJumpers(PreferencesType *prefs) 
{
  // only do this if the player is still alive
  if (!gbls->playerDied) {

    SHORT i, j;

    // process the jumpers
    i = 0;
    while (i<prefs->game.phire.jumperCount) {

      SHORT   bounceIndex[] = { 10, 18, 24 };
      BOOLEAN removal       = FALSE;

      if (prefs->game.phire.jumperWait[i] == 0) {

        BOOLEAN ok;

        // lets make sure it is not moving into a jumper in front of us?
        ok = TRUE;
        for (j=0; j<prefs->game.phire.jumperCount; j++) {

          ok &= (
                 (prefs->game.phire.jumperPosition[i]+1 !=
                  prefs->game.phire.jumperPosition[j])
                );
        }

        // the coast is clear, move!
        if (ok) {

          // CASE 1: has the jumper hit the floor?
	  for (j=0; j<3; j++) {

            if (prefs->game.phire.jumperPosition[i] == bounceIndex[j]) {

              // trampoline was not there, death
	      if (prefs->game.phire.trampolinePosition != j) {

                gbls->playerDied |= TRUE;
		prefs->game.phire.jumperDeathPosition = j;

		// remove the jumper
		GameRemoveJumper(prefs, i); removal = TRUE;
	      }

              // trampoline was there, score 
	      else
	        GameIncrementScore(prefs);

              break; // get out of loop
	    }
	  }

          // CASE 2: has the jumper made it to the ambulance?
          if ((prefs->game.phire.jumperPosition[i] == 27)) {

            // remove the jumper
	    GameRemoveJumper(prefs, i); removal = TRUE;
	  }

          // did we not have to remove it?
	  if (!removal) {

            if (prefs->game.phire.jumperPosition[i] < 7)
	      prefs->game.phire.jumperPosition[i] += 6;
	    prefs->game.phire.jumperPosition[i]++;

            prefs->game.phire.jumperWait[i] =
              (gbls->gameType == GAME_A) ? 6 : 4;
            gbls->jumperChanged[i] = TRUE;
 
            // play a movement sound
            SndPlaySndEffect(SNDRES1_BEEP);
          }
        }
      }
      else {

        prefs->game.phire.jumperWait[i]--;

        // has the jumper been "bounced" off?
        for (j=0; j<3; j++) {

          if (
              (prefs->game.phire.jumperPosition[i] == bounceIndex[j]) &&
              (prefs->game.phire.trampolinePosition == j)
             ) {

            // increase score 
 	    GameIncrementScore(prefs);

            // move the jumper along
	    prefs->game.phire.jumperPosition[i]++;
            prefs->game.phire.jumperWait[i] =
              (gbls->gameType == GAME_A) ? 6 : 4;
            gbls->jumperChanged[i] = TRUE;
          }
        }
      }

      if (!removal) i++;
    }

    // new jumper appearing on screen?
    {
      BOOLEAN ok;
      BYTE    birthFactor        = (gbls->gameType == GAME_A) ? 8 : 4;
      BYTE    maxOnScreenJumpers = prefs->game.phire.gameLevel;

      // we must be able to add a jumper (based on level)
      ok = (
            (prefs->game.phire.jumperCount < maxOnScreenJumpers) &&
            ((DeviceRandom(0) % birthFactor) == 0)
           );

      // lets check that there are not any jumpers alread :)
      for (i=0; i<prefs->game.phire.jumperCount; i++) {
        ok &= (prefs->game.phire.jumperPosition[i] > 8);
      }

      // lets add a new parachuter
      if (ok) {

        BYTE new, pos;

        if (gbls->gameType == GAME_A) new = 0;
        else                          new = DeviceRandom(0) % 2;

        pos = prefs->game.phire.jumperCount++;
        prefs->game.phire.jumperPosition[pos] = new;
        prefs->game.phire.jumperWait[pos]     = 
          (gbls->gameType == GAME_A) ? 6 : 4;
        gbls->jumperChanged[pos]              = TRUE;
        gbls->jumperOnScreen[pos]             = FALSE;
        gbls->jumperOnScreenPosition[pos]     = 0;

        // play a movement sound
        SndPlaySndEffect(SNDRES1_BEEP);
      }
    }
  }
}

/**
 * Remove a jumper from the game.
 *
 * @param prefs the global preference data.
 * @param jumperIndex the index of the jumper to remove.
 */
static void 
GameRemoveJumper(PreferencesType *prefs, 
                 SHORT           jumperIndex)                  
{
  SHORT     index;
  GfxRegion region    = { {   0,   0 }, {   0,   0 } };
  GfxRegion scrRegion = { {   0,   0 }, {   0,   0 } };

  // 
  // remove the bitmap from the screen
  //
 
  if (gbls->jumperOnScreen[jumperIndex]) {

    index = gbls->jumperOnScreenPosition[jumperIndex];

    // what is the rectangle we need to copy?
    GameGetSpritePosition(spriteJumper, index,
                          &scrRegion.topLeft.x, &scrRegion.topLeft.y);
    scrRegion.extent.x  = 14;
    scrRegion.extent.y  = 14;
    region.topLeft.x    = (index % 7) * scrRegion.extent.x;
    region.topLeft.y    = (index / 7) * scrRegion.extent.y;
    region.extent.x     = scrRegion.extent.x;
    region.extent.y     = scrRegion.extent.y;

    // invert the old jumper bitmap!
    GfxCopyRegion(gbls->winJumpers, GfxGetDrawWindow(),
                  &region, scrRegion.topLeft.x, scrRegion.topLeft.y, gfxMask);
  }

  //
  // update the information arrays
  //

  // we will push the 'jumper' out of the array
  //
  // before: 1234567---  after: 1345672---
  //          ^     |                 |
  //                end point         end point

  prefs->game.phire.jumperCount--;

  // removal NOT from end?
  if (prefs->game.phire.jumperCount > jumperIndex) {

    SHORT i, count;

    count = prefs->game.phire.jumperCount - jumperIndex;

    // shift all elements down
    for (i=jumperIndex; i<(jumperIndex+count); i++) {
      prefs->game.phire.jumperPosition[i] = prefs->game.phire.jumperPosition[i+1];
      prefs->game.phire.jumperWait[i]     = prefs->game.phire.jumperWait[i+1];
      gbls->jumperChanged[i]              = gbls->jumperChanged[i+1];
      gbls->jumperOnScreen[i]             = gbls->jumperOnScreen[i+1];
      gbls->jumperOnScreenPosition[i]     = gbls->jumperOnScreenPosition[i+1];
    }
  }
}
