//
//  BubbleSprite.h
//  t06
//
//  Created by 野口一也 on 2014/06/24.
//
//

#ifndef __t06__BubbleSprite__
#define __t06__BubbleSprite__

#include "cocos2d.h"
#include "Constants.h"

USING_NS_CC;

class BubbleSprite : public Sprite
{
public:
    enum BubbleState{
        StateStatic,
        StateMoving,
        StateDisable
    };
    
    BubbleSprite();
    virtual void update(float delta);
    void backToPreviousPosition();
    void halfUpdate(float delta);
    
    static BubbleSprite* createWithSpriteFrame(SpriteFrame *spriteFrame);
    static BubbleSprite* createWithSpriteFrameName(const std::string& spriteFrameName);
    CC_SYNTHESIZE(Vec2, _direction, Direction);
    CC_SYNTHESIZE(BubbleState, _state, State);
    CC_SYNTHESIZE(bool, _mustBeDestroyed, MustBeDestroyed);
    CC_SYNTHESIZE(bool, _mustBeHeld, MustBeHeld);
private:
    CC_SYNTHESIZE(int, _typeBubble, TypeBubble);
    CC_SYNTHESIZE(Vec2, _oldPOsition, OldPosition);
};

#endif /* defined(__t06__BubbleSprite__) */
