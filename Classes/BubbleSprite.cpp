//
//  BubbleSprite.cpp
//  t06
//
//  Created by 野口一也 on 2014/06/24.
//
//

#include "BubbleSprite.h"

BubbleSprite::BubbleSprite()
: _typeBubble(-1)
, _direction(Vec2::ZERO)
, _state(BubbleState::StateDisable)
, _mustBeDestroyed(false)
, _mustBeHeld(false)
, _oldPOsition(Vec2::ZERO)
{
    
}

BubbleSprite* BubbleSprite::createWithSpriteFrame(SpriteFrame *spriteFrame)
{
    BubbleSprite *sprite = new BubbleSprite();
    if (spriteFrame && sprite && sprite->initWithSpriteFrame(spriteFrame))
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

BubbleSprite* BubbleSprite::createWithSpriteFrameName(const std::string& spriteFrameName)
{
    SpriteFrame *frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(spriteFrameName);
    
#if COCOS2D_DEBUG > 0
    char msg[256] = {0};
    sprintf(msg, "Invalid spriteFrameName: %s", spriteFrameName.c_str());
    CCASSERT(frame != nullptr, msg);
#endif
    
    return createWithSpriteFrame(frame);
}

void BubbleSprite::update(float delta)
{
    if (_state == BubbleSprite::BubbleState::StateMoving) {
        float force_x = _direction.x * BUBBLE_VELOCITY * delta;
        float force_y = _direction.y * BUBBLE_VELOCITY * delta;
        float newX = this->getPositionX() + force_x;
        float newY = this->getPositionY() + force_y;
        _oldPOsition = this->getPosition();
        this->setPosition(Point(newX, newY));
    }
}

void BubbleSprite::backToPreviousPosition()
{
    this->setPosition(_oldPOsition);
}

void BubbleSprite::halfUpdate(float delta)
{
    float force_x = _direction.x * (BUBBLE_VELOCITY / 2) * delta;
    float force_y = _direction.y * (BUBBLE_VELOCITY / 2) * delta;
    float newX = this->getPositionX() + force_x;
    float newY = this->getPositionY() + force_y;
    _oldPOsition = this->getPosition();
    this->setPosition(Point(newX, newY));
}
