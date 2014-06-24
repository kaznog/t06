//
//  Field.h
//  t06
//
//  Created by 野口一也 on 2014/06/24.
//
//

#ifndef __t06__Field__
#define __t06__Field__

#include "cocos2d.h"
#include "Constants.h"
#include "BubbleSprite.h"
#include "AimHelper.h"
USING_NS_CC;

typedef struct {
    float t;
    int bx, by;
} HitInfo;

class Field : public Layer
{
public:
    Field();
    ~Field();
    enum {
        kPlaying,
        kPause,
        kGameOver,
    };
    std::vector<std::string> blocklist = {
        "bubble_0.png",
        "bubble_1.png",
        "bubble_2.png",
        "bubble_3.png",
        "bubble_4.png",
        "block_buzz_s.png",
        "block_daisy_s.png",
        "block_dale_s.png",
        "block_donald_s.png",
        "block_dumbo_s.png",
        "block_eeyore_s.png",
        "block_mickey_s.png",
    };
    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();
    
    virtual void update(float delta);
    // touch delegate
    virtual bool onTouchBegan(Touch *touch, Event *event);
    virtual void onTouchMoved(Touch *touch, Event *event);
    virtual void onTouchEnded(Touch *touch, Event *event);
    virtual void onTouchCancelled(Touch *touch, Event *event);
    // implement the "static create()" method manually
    CREATE_FUNC(Field);
    
    CC_SYNTHESIZE(float, _scalew, ScaleWidth);
    CC_SYNTHESIZE(float, _scaleh, ScaleHeight);
    CC_SYNTHESIZE(float, _block_scale_width, BlockScaleWidth);
    CC_SYNTHESIZE(float, _block_scale_height, BlockScaleHeight);
    CC_SYNTHESIZE(float, _basey, BaseY);
    CC_SYNTHESIZE(float, _basey_offset, BaseYOffset);
    CC_SYNTHESIZE(float, _scaled_height, ScaledHeight);
    CC_SYNTHESIZE(float, _scaled_half_height, ScaledHalfHeight);
    CC_SYNTHESIZE(float, _scaled_width, ScaledWidth);
    CC_SYNTHESIZE(float, _scaled_half_width, ScaledHalfWidth);
    
    CC_SYNTHESIZE(Size, _winSize, WinSize);
    
    CC_SYNTHESIZE(Vec2, _shotPoint, ShotPoint);
    CC_SYNTHESIZE(Vec2, _nextShotPoint, NextShotPoint);
    CC_SYNTHESIZE(Vec2, _lastPoint, LastPoint);
    
    CC_SYNTHESIZE(int, _pushedRows, PushedRows);
    CC_SYNTHESIZE(bool, _shooting, Shooting);
    CC_SYNTHESIZE(bool, _even, Even);
    CC_SYNTHESIZE(int, _filledRows, FilledRows);
    CC_SYNTHESIZE(float, _elapsedTime, ElapsedTime);
    
    CC_SYNTHESIZE(AimHelper*, _aimHelper, AimHelper);
    
private:
    int _state;
    BubbleSprite* _field[FIELD_WIDTH * FIELD_HEIGHT];
    std::vector< std::vector<BubbleSprite*> >* bubblesGrid;
    BubbleSprite* _next;
    BubbleSprite* bubbleToShoot;
    
    void startGame();
    void restartGame();
    void fillGrid(int _numberOfRows);
    void addNewRow();
    void addNewRowAtIndex(int _idx);
    void pushRows();
    void prepareBubbleToShoot();
    void prepareNextBubbleToShoot();
    void shootBubble();
    
    bool detectGameOver();
    void setGameOver();
    void updateEffects();
    int chooseNextBubble();
    void updateNextBubble();
    Sprite* _gameoverline;
    
    /** Touch listener */
    void enableTouchEvent(bool enabled);
    CC_SYNTHESIZE(EventListenerTouchOneByOne*, _touchListener, TouchListener);
    
    void checkWallCollisions();
    BubbleSprite* checkBubblesCollisions();
    
    Vec2 getPositionForRow(int _row, int _col);
    void updateNumberOfRows();
    int checkMatchesColor(int _color, int _row, int _col);
    void destroyBubbles();
    void clearBubbles();
    void checkAndDropBubbles();
    Vec2 getNearestEmptySlotForRow(int _row, int _col);
    Vec2 getNearestEmptySlotForRow(BubbleSprite* colliedBubble);
    void holdBubbleForGridRow(int _row, int _col);
};

#endif /* defined(__t06__Field__) */
