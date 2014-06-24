//
//  Field.cpp
//  t06
//
//  Created by 野口一也 on 2014/06/24.
//
//

#include "Field.h"

Field::Field()
: _state(kPause)
, _pushedRows(0)
, _shooting(false)
, _even(true)
, _elapsedTime(0.0f)
, _aimHelper(nullptr)
{
    
}

Field::~Field()
{
    enableTouchEvent(false);
}

bool Field::init()
{
    if (!Layer::init()) {
        return false;
    }
    
    auto director = Director::getInstance();
    _winSize = director->getWinSize();
    _scalew = _winSize.width/ 320.0f;
    _scaleh = _winSize.height/480.0f;
    _block_scale_width = ((_winSize.width - (HORIZONTAL_OFFSET*2)) / FIELD_WIDTH) / BLOCK_ORIGINAL_WIDTH;
    float hl = MIN(_winSize.height, 480.0f);
    float hh = MAX(_winSize.height, 480.0f);
    _basey = hl;
    _basey_offset = hh - hl;
    _scaled_width = BLOCK_ORIGINAL_WIDTH * _block_scale_width;
    _scaled_half_width = _scaled_width / 2;
    _scaled_height = (_scaled_width / 2) * sqrtf(3);
    _scaled_half_height = _scaled_height / 2;
    
    auto spriteCache = SpriteFrameCache::getInstance();
    spriteCache->addSpriteFramesWithFile("atlas.plist");
    
    bubblesGrid = new std::vector< std::vector<BubbleSprite*> >(MAX_ROWS);
    for (int i = 0; i < MAX_ROWS; i++) {
        auto rows = new std::vector<BubbleSprite*>(FIELD_WIDTH);
        for (int j = 0; j < FIELD_WIDTH; j++) {
            rows->at(j) = nullptr;
        }
        bubblesGrid->at(i) = *rows;
    }
    
    Vec2 shotbase = getPositionForRow(17, 6);
    _shotPoint.setPoint(_winSize.width/2, shotbase.y);
    _nextShotPoint = getPositionForRow(13, 1);
    
    spriteCache->addSpriteFramesWithFile("gameoverline_texture.plist");
    _gameoverline = Sprite::createWithSpriteFrameName("gameoverline001.png");
    auto cache = AnimationCache::getInstance();
    cache->addAnimationsWithFile("gameoverline.plist");
    auto animation = cache->getAnimation("gameoverline");
    auto action = Animate::create(animation);
    _gameoverline->runAction(action);
    _gameoverline->setPosition(Vec2(_winSize.width/2, getPositionForRow(MAX_ROWS - 1, 6).y));
    _gameoverline->setTag(1000);
    this->addChild(_gameoverline, 100);
    
    // AimHelperの初期化
    _aimHelper = AimHelper::create();
    _aimHelper->setOrigin(_shotPoint);
    _aimHelper->setVisible(false);
    this->addChild(_aimHelper, ZORDER_AIMHELPER);
    
    enableTouchEvent(true);
    
    startGame();
    return true;
}

#pragma mark Game

void Field::startGame()
{
    _pushedRows = 0;
    _filledRows = 0;
    _shooting = false;
    _even = true;
    _elapsedTime = 0.0f;
    fillGrid(5);
    prepareBubbleToShoot();
    
    // Set game as playing
    _state = kPlaying;
}

void Field::restartGame()
{
    // Remove all bubbles from grid
    for (std::vector< std::vector<BubbleSprite*> >::iterator it = bubblesGrid->begin(); it < bubblesGrid->end(); it++) {
        std::vector<BubbleSprite*> rows = *it;
        for (int j = 0; j < FIELD_WIDTH; j++) {
            BubbleSprite* bubble = rows.at(j);
            if (bubble != nullptr) {
                bubble->removeFromParentAndCleanup(true);
                rows.at(j) = nullptr;
            }
        }
        *it = rows;
    }
    
    // Remove shooting bubble, if any
    if (bubbleToShoot != nullptr) {
        bubbleToShoot->removeFromParentAndCleanup(true);
        bubbleToShoot = nullptr;
    }

    auto label = this->getChildByTag(TAG_GAMEOVER_LABEL);
    label->removeFromParentAndCleanup(true);
    
    // Start another game
    startGame();
}

void Field::setGameOver()
{
    if (_state == kPlaying) {
        // Show game over label and stop director
        auto label = LabelTTF::create("Game Over", "Arial", 64);
        label->setPosition(_winSize.width/2, _winSize.height/2);
        label->setTag(TAG_GAMEOVER_LABEL);
        this->addChild(label, ZORDER_GAMEOVER_LABEL);
        
        _state = kGameOver;
    }
}

void Field::updateEffects()
{
}


#pragma mark -
#pragma mark Update

void Field::update(float delta)
{
    switch (_state) {
        case kPause:
            break;
        case kPlaying:
        {
            _elapsedTime += delta;
            if (_shooting) {
                if (bubbleToShoot != nullptr) {
                    bubbleToShoot->update(delta);
                    checkWallCollisions();
                    BubbleSprite* collideBubble = checkBubblesCollisions();;
                    if (collideBubble != nullptr) {
                        int x = 0, y = 0, rowOffset = 0, numRow = 0;
                        float distance = 0;
                        Vec2 u = bubbleToShoot->getDirection();
                        u = Vec2(-u.x, -u.y);
                        bubbleToShoot->setDirection(u);
                        bubbleToShoot->setState(BubbleSprite::BubbleState::StateMoving);
                        do {
                            bubbleToShoot->update(delta);
                            checkWallCollisions();
                            distance = bubbleToShoot->getPosition().getDistance(collideBubble->getPosition());
                        } while (distance < _scaled_width * 0.9f);
                        bubbleToShoot->setState(BubbleSprite::BubbleState::StateStatic);
                        // Calculate first the row it's going to fit in
                        y = (_winSize.height - (bubbleToShoot->getPositionY() + (_scaled_height * -1))) / _scaled_height;
                        
                        // Calculate if this row has an offset
                        int CHECK_WIDTH = FIELD_WIDTH;
                        rowOffset = 0;
                        numRow = _even ? y : y + 1;
                        if (numRow % 2 != 0) {
                            log("奇数行なので右へ１つズラすオフセットを準備");
                            rowOffset = _scaled_width / 2;
                            CHECK_WIDTH -= 1;
                        }
                        // Now calculate the column on the grid
                        x = (bubbleToShoot->getPositionX() - (HORIZONTAL_OFFSET * _block_scale_width) + rowOffset) / _scaled_width;
                        
                        Vec2 newSlot = getNearestEmptySlotForRow(collideBubble);
                        x = newSlot.x;
                        y = newSlot.y;
                        
                        // Reposition correctly
                        // Add it to grid
                        std::vector<BubbleSprite*> rerow = bubblesGrid->at(y);
                        rerow.at(x) = bubbleToShoot;
                        bubblesGrid->at(y) = rerow;
                        
                        updateNumberOfRows();
                        
                        bubbleToShoot->setPosition(getPositionForRow(y, x));
                        
                        // Check color matches
                        if (checkMatchesColor(bubbleToShoot->getTypeBubble(), y, x) >= NUM_OF_MATCHES_REQUIRED) {
                            destroyBubbles();
                            checkAndDropBubbles();
                        }else{
                            clearBubbles();
                        }
                        
                        log("update bubble shoot to static start");
                        // Remove reference
                        log("prepare next shoot bubble");
                        bubbleToShoot = nullptr;
                        prepareBubbleToShoot(); // Prepare another bubble
                    }
                }
            } else {
                if (_elapsedTime > TIME_TO_PUSHROW || _elapsedTime < 0.0f) {
                    pushRows();
                    addNewRow();
                    
                    _elapsedTime = 0;
                }
            }
        }
            break;
        case kGameOver:
            break;
        default:
            break;
    }
    updateNextBubble();
    updateEffects();
}

#pragma mark -
#pragma mark Grid

void Field::fillGrid(int _numberOfRows)
{
    int count = 0;
    do {
        addNewRowAtIndex(count);
        count++;
    } while (count < _numberOfRows);
}

void Field::addNewRow()
{
    // Adds row at index 0 by default
    addNewRowAtIndex(0);
}

void Field::addNewRowAtIndex(int _idx)
{
    // Calculate number of bubbles based on position or number of rows
    int numberOfBubbles = FIELD_WIDTH - 1;
    
    // Check wheter pushing a row, or populating at the beginning
    if (_idx == 0) {
        // User number of pushed rows instead
        if (_pushedRows % 2 != 0) {
            numberOfBubbles = FIELD_WIDTH;
        }
    }else{
        // Use index
        if (_idx % 2 != 0) {
            numberOfBubbles = FIELD_WIDTH;
        }
    }
    BubbleSprite* bubble = nullptr;
    auto row = new std::vector<BubbleSprite*>(FIELD_WIDTH);
    for (int i = 0; i < numberOfBubbles; i++) {
        int charactor = arc4random_uniform(BLOCK_PATTERN_MAX);
        std::string charactorname = blocklist[charactor];
        bubble = BubbleSprite::createWithSpriteFrameName(charactorname);
        bubble->setState(BubbleSprite::BubbleState::StateStatic);
        bubble->setPosition(getPositionForRow(_idx, i));
        bubble->setScale(_block_scale_width);
        bubble->setTag(10);
        bubble->setTypeBubble(charactor);
        
        this->addChild(bubble, 10);
        //        }
        row->at(i) = bubble;
    }
    // For rows with 11 items, last item is null
    if (numberOfBubbles < FIELD_WIDTH) {
        row->at(FIELD_WIDTH - 1) = nullptr;
    }
    bubblesGrid->insert(bubblesGrid->begin() + _idx, *row);
    _pushedRows++; // Updates number of pushed rows
    updateNumberOfRows(); // Updates total number of rows with at least one bubble
    
    // If grid has more than allowed rows, remove
    if (bubblesGrid->size() > MAX_ROWS) {
        //        bubblesGrid->pop_back();
        std::vector<BubbleSprite*> lastRow = bubblesGrid->back();
        std::vector<BubbleSprite*>::iterator it = lastRow.begin();
        while (it != lastRow.end()) {
            BubbleSprite* bubble = *it;
            if (bubble != nullptr) {
                bubble->removeFromParentAndCleanup(true);
            }
            it++;
        }
        bubblesGrid->pop_back();
    }
}

void Field::pushRows()
{
    // Shift all bubbles in each row to the next one
    updateNumberOfRows();
    
    for (int i = _filledRows - 1; i >= 0; i--) {
        std::vector<BubbleSprite*> row = bubblesGrid->at(i);
        
        // Move physically every bubble in this row
        for (std::vector<BubbleSprite*>::iterator it = row.begin(); it != row.end(); it++) {
            BubbleSprite* bubble = (BubbleSprite*)*it;
            if (bubble != nullptr) {
                bubble->runAction(MoveBy::create(0.2f, Vec2(0, - _scaled_height)));
            }
        }
    }
    
    // Switch even flag
    _even = !_even;
}

void Field::updateNumberOfRows()
{
    int newNumberOfFilledRows = 0;
    for (std::vector< std::vector<BubbleSprite*> >::iterator it = bubblesGrid->begin(); it < bubblesGrid->end(); it++) {
        std::vector<BubbleSprite*> rows = *it;
        for (int j = 0; j < FIELD_WIDTH; j++) {
            if (rows.at(j) != nullptr) {
                newNumberOfFilledRows++;
                break;
            }
        }
    }
    _filledRows = newNumberOfFilledRows;
    // Check end of game
    if (_filledRows >= MAX_ROWS) {
        setGameOver();
    }
}

Vec2 Field::getPositionForRow(int _row, int _col)
{
    // Calculate position in layer based on offset and bubble size
    float y = _winSize.height - (_scaled_height * -1) - _scaled_half_height - _row * _scaled_height; // Fill downwards
    float x = (HORIZONTAL_OFFSET * _block_scale_width) + _scaled_width / 2 + _col * _scaled_width;
    
    // Check row is indented
    _row = _even ? _row : _row + 1;
    if (_row % 2 != 0) {
        x -= _scaled_width / 2;
    }
    
    return Vec2(x, y);
}

int Field::checkMatchesColor(int _color, int _row, int _col)
{
    log("Call to matches Color row %d col %d", _row, _col);
    // Recursive function to check color matches on all bubbles nearby
    int colorMatches = 0;
    
    // Obtain upper, middle and lower rows
    std::vector<BubbleSprite*>* upperRow  = _row > 1 ? &(bubblesGrid->at(_row - 1)) : nullptr;
    std::vector<BubbleSprite*>* middleRow = &(bubblesGrid->at(_row));
    std::vector<BubbleSprite*>* lowerRow  = _row < (MAX_ROWS - 1) ? &(bubblesGrid->at(_row + 1)) : nullptr;
    
    // Get color to look for
    BubbleSprite* thisBubble = middleRow->at(_col);
    if (thisBubble == nullptr) {
        return 0;
    }
    // Check if color matches
    if (thisBubble->getTypeBubble() == _color && !thisBubble->getMustBeDestroyed()) {
        colorMatches++;
        thisBubble->setMustBeDestroyed(true);
    }
    
    // Check upper bubble, if any
    if (upperRow != nullptr) {
        BubbleSprite* upperBubble = upperRow->at(_col);
        if (upperBubble != nullptr) {
            if (!upperBubble->getMustBeDestroyed() && upperBubble->getTypeBubble() == _color) {
                colorMatches += checkMatchesColor(_color, _row - 1, _col);
            }
        }
    }
    
    // Check lower bubbles, if any
    if (lowerRow != nullptr) {
        BubbleSprite* lowerBubble = lowerRow->at(_col);
        if (lowerBubble != nullptr) {
            if (!lowerBubble->getMustBeDestroyed() && lowerBubble->getTypeBubble() == _color) {
                colorMatches += checkMatchesColor(_color, _row + 1, _col);
            }
        }
    }
    
    // Check if it's an even row to know nearby bubbles
    if ((_row -  _pushedRows) % 2 != 0) {
        log("Check even row(%d) nearby bubbles col + 1(%d) start", _row, _col + 1);
        // NOTE: col +1
        
        // Check upper right and lower right bubbles
        if (upperRow != nullptr && _col < FIELD_WIDTH - 1) {
            BubbleSprite* upRight = upperRow->at(_col + 1);
            if (upRight != nullptr) {
                if (!upRight->getMustBeDestroyed() && upRight->getTypeBubble() == _color) {
                    colorMatches += checkMatchesColor(_color, _row - 1, _col + 1);
                }
            }
        }
        
        if (lowerRow != nullptr && _col < FIELD_WIDTH - 1) {
            BubbleSprite* lowerRight = lowerRow->at(_col + 1);
            if (lowerRight != nullptr) {
                if (!lowerRight->getMustBeDestroyed() && lowerRight->getTypeBubble() == _color) {
                    colorMatches += checkMatchesColor(_color, _row + 1, _col + 1);
                }
            }
        }
        log("Check even row(%d) nearby bubbles col + 1(%d) end", _row, _col + 1);
    } else {
        // NOTE: col -1
        
        // Check upper left and lower left bubbles
        if (upperRow != nullptr && _col > 0) {
            BubbleSprite* upLeft = upperRow->at(_col - 1);
            if (upLeft != nullptr) {
                if (!upLeft->getMustBeDestroyed() && upLeft->getTypeBubble() == _color) {
                    colorMatches += checkMatchesColor(_color, _row - 1, _col - 1);
                }
            }
        }
        
        if (lowerRow != nullptr && _col > 0) {
            BubbleSprite* lowerLeft = lowerRow->at(_col - 1);
            if (lowerLeft != nullptr) {
                if (!lowerLeft->getMustBeDestroyed() && lowerLeft->getTypeBubble() == _color) {
                    colorMatches += checkMatchesColor(_color, _row + 1, _col - 1);
                }
            }
        }
    }
    // Check left nearby bubble
    if (_col > 0) {
        BubbleSprite* left = middleRow->at(_col - 1);
        if (left != nullptr) {
            if (!left->getMustBeDestroyed() && left->getTypeBubble() == _color) {
                colorMatches += checkMatchesColor(_color, _row, _col - 1);
            }
        }
    }
    
    
    // Check right nearby bubble
    if (_col < FIELD_WIDTH - 1) {
        BubbleSprite* right = middleRow->at(_col + 1);
        if (right != nullptr) {
            if (!right->getMustBeDestroyed() && right->getTypeBubble() == _color) {
                colorMatches += checkMatchesColor(_color, _row, _col + 1);
            }
        }
    }
    
    return colorMatches;
}

void Field::destroyBubbles()
{
    for (std::vector< std::vector<BubbleSprite*> >::iterator it = bubblesGrid->begin(); it != bubblesGrid->end(); it++) {
        std::vector<BubbleSprite*> row = *it;
        for (int col = 0; col < FIELD_WIDTH; col++) {
            BubbleSprite* bubble = row.at(col);
            if (bubble != nullptr && bubble->getMustBeDestroyed()) {
                // Remove from grid
                row.at(col) = nullptr;
                *it = row;
                auto scaleAction = ScaleTo::create(0.2f, 0.01f);
                auto func = CallFunc::create([bubble]() {
                    bubble->removeFromParentAndCleanup(true);
                });
                bubble->runAction(Sequence::create(scaleAction, func, NULL));
            }
        }
        *it = row;
    }
}

void Field::clearBubbles()
{
    // Reset all bubbles to not be destroyed
    for (std::vector< std::vector<BubbleSprite*> >::iterator it = bubblesGrid->begin(); it != bubblesGrid->end(); it++) {
        std::vector<BubbleSprite*> row = *it;
        for (int col = 0; col < FIELD_WIDTH; col++) {
            BubbleSprite* bubble = row.at(col);
            if (bubble != nullptr) {
                bubble->setMustBeDestroyed(false);
            }
            row.at(col) = bubble;
        }
        *it = row;
    }
}

void Field::checkAndDropBubbles()
{
    // Recursive call to hold bubbles that are linked
    holdBubbleForGridRow(0, 0);
    
    // Set not held bubbles to drop
    for (std::vector< std::vector<BubbleSprite*> >::iterator it = bubblesGrid->begin(); it != bubblesGrid->end(); it++) {
        std::vector<BubbleSprite*> row = *it;
        for (int col = 0; col < FIELD_WIDTH; col++) {
            BubbleSprite* bubble = row.at(col);
            if (bubble != nullptr) {
                if (!bubble->getMustBeHeld()) {
                    row.at(col) = nullptr;
                    *it = row;
                    auto easeout = EaseOut::create(MoveBy::create(0.7f, Vec2(0, - _winSize.height)), 0.5f);
                    auto func = CallFunc::create([bubble]() {
                        bubble->removeFromParentAndCleanup(true);
                    });
                    bubble->runAction(Sequence::create(easeout,
                                                       func,
                                                       NULL));
                } else {
                    bubble->setMustBeHeld(false);
                    row.at(col) = bubble;
                }
            }
        }
        *it = row;
    }
}

Vec2 Field::getNearestEmptySlotForRow(BubbleSprite* colliedBubble)
{
    Vec2 targetPos = colliedBubble->getPosition();
    Vec2 shootBubblePos = bubbleToShoot->getPosition();
    // Calculate first the row it's going to fit in
    int y = (_winSize.height - (targetPos.y + (_scaled_height * -1))) / _scaled_height;
    
    // Calculate if this row has an offset
    int rowOffset = 0;
    int numRow = _even ? y : y + 1;
    if (numRow % 2 != 0) {
        log("奇数行なので右へ１つズラすオフセットを準備");
        rowOffset = _scaled_width / 2;
    }
    // Now calculate the column on the grid
    int x = (targetPos.x - (HORIZONTAL_OFFSET * _block_scale_width) + rowOffset) / _scaled_width;
    
    std::vector<BubbleSprite*>* upperRow  = y > 1 ? &(bubblesGrid->at(y - 1)) : nullptr;
    std::vector<BubbleSprite*>* middleRow = &(bubblesGrid->at(y));
    std::vector<BubbleSprite*>* lowerRow  = bubblesGrid->size() <= y + 1 ? nullptr : y < (MAX_ROWS - 1) ? &(bubblesGrid->at(y + 1)) : nullptr;
    
    float angle = CC_RADIANS_TO_DEGREES( atan2((shootBubblePos.x - targetPos.x), (shootBubblePos.y - targetPos.y)) );
    log("接触角度:%f", angle);
    float block = std::abs(angle/60);
    log("判定block:%f", block);
    if (angle < 0) {
        log("左半分判定");
        if (numRow % 2 != 0) {
            log("奇数行の場合の判定");
            if (x > 0 && block < 1.0f && upperRow != nullptr) {
                log("upperRow col - 1判定");
                BubbleSprite* bubble  = upperRow->at(x - 1);
                if (bubble == nullptr) {
                    return Vec2(x - 1, y - 1);
                }
            } else if (x > 0 && block < 2.0f && middleRow != nullptr) {
                log("middlwRow col - 1判定");
                BubbleSprite* bubble = middleRow->at(x - 1);
                if (bubble == nullptr) {
                    return Vec2(x - 1, y);
                }
            } else if (x > 0 && block <= 3.0f && lowerRow != nullptr) {
                log("lowerRow col - 1判定");
                BubbleSprite* bubble = lowerRow->at(x - 1);
                if (bubble == nullptr) {
                    return Vec2(x - 1, y + 1);
                } else {
                    log("対象見つからず");
                }
            } else if (x == 0 && block <= 3.0f && lowerRow != nullptr) {
                BubbleSprite* bubble = lowerRow->at(x);
                if (bubble == nullptr) {
                    return Vec2(x, y + 1);
                }
            }
        } else {
            log("偶数行の場合の判定");
            if (x >= 0 && block < 1.0f && upperRow != nullptr) {
                log("upperRow col判定");
                BubbleSprite* bubble = upperRow->at(x);
                if (bubble == nullptr) {
                    return Vec2(x, y - 1);
                }
            } else if (x > 0 && block < 2.0f && middleRow != nullptr) {
                log("middleRow col + 1判定");
                BubbleSprite* bubble = middleRow->at(x - 1);
                if (bubble == nullptr) {
                    return Vec2(x - 1, y);
                }
            } else if (x >= 0 && block <= 3.0f && lowerRow != nullptr) {
                log("lowerRow col + 1判定");
                BubbleSprite* bubble = lowerRow->at(x);
                if (bubble == nullptr) {
                    return Vec2(x, y + 1);
                } else {
                    log("対象見つからず");
                }
            }
        }
    } else {
        log("右半分判定");
        if (numRow % 2 != 0) {
            log("奇数行の場合の判定");
            if (x < FIELD_WIDTH - 1 && block < 1.0f && upperRow != nullptr) {
                log("upperRow col判定 オフセットあり行の下なのでオフセットあり");
                BubbleSprite* bubble = upperRow->at(x);
                if (bubble == nullptr) {
                    return Vec2(x, y - 1);
                }
            } else if (x < FIELD_WIDTH - 1 && block < 2.0f && middleRow != nullptr) {
                log("middleRow col + 1判定");
                BubbleSprite* bubble = middleRow->at(x + 1);
                if (bubble == nullptr) {
                    return Vec2(x + 1, y);
                }
            } else if (x < FIELD_WIDTH - 1 && block <= 3.0f && lowerRow != nullptr) {
                log("lowerRow col判定 オフセットなし行の下なのでオフセットあり");
                BubbleSprite* bubble = lowerRow->at(x);
                if (bubble == nullptr) {
                    return Vec2(x, y + 1);
                } else {
                    log("対象見つからず");
                }
            } else if (x == FIELD_WIDTH - 1 && block <= 3.0f && lowerRow != nullptr) {
                BubbleSprite* bubble = lowerRow->at(x - 1);
                if (bubble == nullptr) {
                    return Vec2(x - 1, y + 1);
                } else {
                    std::vector<BubbleSprite*>* lowerlowerRow  = y < (MAX_ROWS - 1) ? &(bubblesGrid->at(y + 2)) : nullptr;
                    if (lowerlowerRow != nullptr) {
                        bubble = lowerlowerRow->at(x);
                        if (bubble == nullptr) {
                            return Vec2(x, y + 2);
                        } else {
                            log("対象見つからず");
                        }
                    } else {
                        log("対象見つからず");
                    }
                }
            }
        } else {
            log("偶数行の場合の判定");
            if (x < FIELD_WIDTH - 2 && block < 1.0f && upperRow != nullptr) {
                log("upperRow col + 1判定");
                BubbleSprite* bubble = upperRow->at(x + 1);
                if (bubble == nullptr) {
                    return Vec2(x + 1, y - 1);
                }
            } else if (x < FIELD_WIDTH - 2 && block < 2.0f && middleRow != nullptr) {
                log("middleRow col判定");
                BubbleSprite* bubble = middleRow->at(x + 1);
                if (bubble == nullptr) {
                    return Vec2(x + 1, y);
                }
            } else if (x < FIELD_WIDTH - 1 && block < 2.0f && lowerRow != nullptr) {
                BubbleSprite* bubble = lowerRow->at(x + 1);
                if (bubble == nullptr) {
                    return Vec2(x + 1, y + 1);
                }
            } else if (x < FIELD_WIDTH - 2 && block <= 3.0f && lowerRow != nullptr) {
                log("lowerRow col 判定");
                BubbleSprite* bubble = lowerRow->at(x + 1);
                if (bubble == nullptr) {
                    return Vec2(x + 1, y + 1);
                } else {
                    log("対象見つからず");
                }
            } else if (x < FIELD_WIDTH - 1 && block <= 3.0f && lowerRow != nullptr) {
                log("lowerRow col 判定");
                BubbleSprite* bubble = lowerRow->at(x + 1);
                if (bubble == nullptr) {
                    return Vec2(x + 1, y + 1);
                } else {
                    log("対象見つからず");
                }
            } else if (x == FIELD_WIDTH - 1 && block <= 3.0f && lowerRow != nullptr) {
                BubbleSprite* bubble = lowerRow->at(x);
                if (bubble == nullptr) {
                    return Vec2(x, y + 1);
                }
            }
        }
    }
    log("想定外で何もに浸からなかった");
    BubbleSprite* bubble = middleRow->at(x);
    if (bubble == nullptr) {
        return Vec2(x, y);
    }
    return Vec2::ZERO;
}


Vec2 Field::getNearestEmptySlotForRow(int _row, int _col)
{
    // Obtain upper, middle and lower rows
    std::vector<BubbleSprite*>* upperRow  = _row > 1 ? &(bubblesGrid->at(_row - 1)) : nullptr;
    std::vector<BubbleSprite*>* middleRow = &(bubblesGrid->at(_row));
    std::vector<BubbleSprite*>* lowerRow  = bubblesGrid->size() <= _row + 1 ? nullptr : _row < (MAX_ROWS - 1) ? &(bubblesGrid->at(_row + 1)) : nullptr;
    
    int newRow = _row;
    int newCol = _col;
    if (_col < 0) {
        log("固定位置検索基準列が範囲オーバーのため、修正");
        newCol = 0;
    }
    if (_col >= FIELD_WIDTH) {
        log("固定位置検索基準列が範囲オーバーのため、修正");
        newCol = FIELD_WIDTH - 1;
    }
    if (_col == newCol) {
        log("現在位置に置けないの？");
        if (middleRow != nullptr) {
            BubbleSprite* bubble = middleRow->at(newCol);
            if (bubble == nullptr) {
                return Vec2(newCol, newRow);
            }
        }
    } else {
        log("横範囲オーバーだったみたいだけど、左右1ずらして置けたの？");
        if (middleRow != nullptr) {
            BubbleSprite* bubble = middleRow->at(newCol);
            if (bubble == nullptr) {
                log("左右調整して置けるみたい");
                return Vec2(newCol, newRow);
            }
        }
    }
    log("上の行、現在行、下の行で設置できる箇所を検索");
    // Check upper bubble, if any
    if (upperRow != nullptr) {
        log("上の行を検索");
        BubbleSprite* upper = upperRow->at(newCol);
        if (upper == nullptr) {
            log("上の行がemptyなので上の行へ");
            return Vec2(newCol, --newRow);
        }
    }
    
    if (newCol > 0) {
        log("現在行の左側を検索");
        BubbleSprite* left = middleRow->at(newCol -1);
        if (left == nullptr) {
            return Vec2(--newCol, newRow);
        }
    }
    // Check right nearby bubble
    if (newCol < FIELD_WIDTH - 1) {
        log("現在行の右側を検索");
        BubbleSprite* right = middleRow->at(newCol + 1);
        if (right == nullptr) {
            return Vec2(++newCol, newRow);
        }
    }
    
    
    log("現在行が奇数行か偶数行かにより検査");
    // Check if it's an even row to know nearby bubbles
    if ((_row - _pushedRows) % 2 != 0) {
        // NOTE: col +1
        log("奇数行の場合の検査");
        log("上の行の右側検査");
        // Check upper right and lower right bubbles
        if (upperRow != nullptr && newCol < FIELD_WIDTH - 1) {
            BubbleSprite* upRight = upperRow->at(newCol + 1);
            if (upRight == nullptr) {
                return Vec2(++newCol, --newRow);
            }
        }
        log("下の行の左側検索");
        if (lowerRow != nullptr && newCol < FIELD_WIDTH - 1) {
            BubbleSprite* lowerRight = lowerRow->at(newCol + 1);
            if (lowerRight == nullptr) {
                return Vec2(--newCol, ++newRow);
            }
        }
    }else{
        // NOTE: col -1
        log("偶数行の場合の検索");
        log("上の業の右側検索");
        // Check upper left and lower left bubbles
        if (upperRow != nullptr && newCol > 0) {
            BubbleSprite* upLeft = upperRow->at(newCol - 1);
            if (upLeft == nullptr) {
                return Vec2(++newCol, --newRow);
            }
        }
        log("下の行の左側検索");
        if (lowerRow != nullptr && newCol > 0) {
            BubbleSprite* lowerLeft = lowerRow->at(newCol - 1);
            if (lowerLeft == nullptr) {
                return Vec2(--newCol, ++newRow);
            }
        }
    }
    
    // Check lower bubbles, if any
    if (lowerRow != nullptr) {
        log("真下の検索");
        BubbleSprite* lower = lowerRow->at(newCol);
        if (lower == nullptr) {
            return Vec2(newCol, ++newRow);
        }
    }
    
    
    log("This shouldn't happen, ever!");
    return Vec2(newCol, newRow);
}

void Field::holdBubbleForGridRow(int _row, int _col)
{
    // Recursive call to hold all adjacent bubbles
    // (Those not held will be dropped thereafter)
    std::vector<BubbleSprite*>* middleRow = &(bubblesGrid->at(_row));
    std::vector<BubbleSprite*>* lowerRow  = _row < (MAX_ROWS - 1) ? &(bubblesGrid->at(_row + 1)) : nullptr;
    
    BubbleSprite* thisBubble = middleRow->at(_col);
    if (thisBubble != nullptr) {
        //        log("row(%d) col(%d)を保持に設定", _row, _col);
        thisBubble->setMustBeHeld(true);
        middleRow->at(_col) = thisBubble;
        bubblesGrid->at(_row) = *middleRow;
    } else {
        //        log("row(%d) col(%d)は存在せず", _row, _col);
    }
    // Check lower bubbles, if any row below
    if (lowerRow != nullptr) {
        BubbleSprite* lower = lowerRow->at(_col);
        if (lower != nullptr) {
            if (!lower->getMustBeHeld()) {
                holdBubbleForGridRow(_row + 1, _col);
            }
        }
        
        // Check lower right or lower left depending if row is even
        if ((_row -  _pushedRows) % 2 != 0) {
            // NOTE: col +1
            if (_col < FIELD_WIDTH - 1) {
                BubbleSprite* lowerRight = lowerRow->at(_col + 1);
                if (lowerRight != nullptr) {
                    if (!lowerRight->getMustBeHeld()) {
                        holdBubbleForGridRow(_row + 1, _col + 1);
                    }
                }
            }
        }else{
            // NOTE: col-1
            
            if (_col > 0) {
                BubbleSprite* lowerLeft = lowerRow->at(_col - 1);
                if (lowerLeft != nullptr) {
                    if (!lowerLeft->getMustBeHeld()) {
                        holdBubbleForGridRow(_row + 1, _col - 1);
                    }
                }
            }
        }
    }
    // Check left nearby bubble
    if (_col > 0) {
        BubbleSprite* left = middleRow->at(_col - 1);
        if (left != nullptr) {
            if (!left->getMustBeHeld()) {
                holdBubbleForGridRow(_row, _col - 1);
            }
        } else {
            //            log("row(%d) col(%d)は存在せず", _row, _col - 1);
        }
    }
    
    // Check right nearby bubble
    if (_col < FIELD_WIDTH - 1) {
        BubbleSprite* right = middleRow->at(_col + 1);
        if (right != nullptr) {
            if (!right->getMustBeHeld()) {
                holdBubbleForGridRow(_row, _col + 1);
            }
        } else {
            //            log("row(%d) col(%d)は存在せず", _row, _col + 1);
        }
    }
    
}

#pragma mark -
#pragma mark Touch Events

bool Field::onTouchBegan(Touch *touch, Event *event)
{
    switch (_state) {
        case kPlaying:
        {
            if (!_shooting) {
                Vec2 location = touch->getLocationInView();
                _lastPoint = Director::getInstance()->convertToGL(location);
                
                if (_lastPoint.y < _shotPoint.y + 20.0f) {
                    _lastPoint.y = _shotPoint.y + 20.0f;
                }
                Vec2 calcPoint = Vec2(_lastPoint.x - _shotPoint.x, _lastPoint.y - _shotPoint.y);
                Vec2 unitary = calcPoint.getNormalized();
                _aimHelper->setDirection(unitary);
                
                float vertivalDistance = (_winSize.height - _shotPoint.y) * _scaled_height;
                _aimHelper->setLength(vertivalDistance / unitary.y);
                
                _aimHelper->setVisible(true);
            }
        } break;
        default:
            break;
    }
    return true;
}

void Field::onTouchMoved(Touch *touch, Event *event)
{
    switch (_state) {
        case kPlaying:
        {
            if (!_shooting) {
                Vec2 location = touch->getLocationInView();
                _lastPoint = Director::getInstance()->convertToGL(location);
                
                if (_lastPoint.y < _shotPoint.y + 20.0f) {
                    _lastPoint.y = _shotPoint.y + 20.0f;
                }
                Vec2 calcPoint = Vec2(_lastPoint.x - _shotPoint.x, _lastPoint.y - _shotPoint.y);
                Vec2 unitary = calcPoint.getNormalized();
                _aimHelper->setDirection(unitary);
                
                float vertivalDistance = (_winSize.height - _shotPoint.y) * _scaled_height;
                _aimHelper->setLength(vertivalDistance / unitary.y);
            }
        } break;
        default:
            break;
    }
}

void Field::onTouchEnded(Touch *touch, Event *event)
{
    switch (_state) {
        case kGameOver:
            restartGame();
            break;
        case kPlaying:
        {
            if (!_shooting) {
                shootBubble();
                _aimHelper->setVisible(false);
            }
        } break;
        default:
            break;
    }
}

void Field::onTouchCancelled(Touch *touch, Event *event)
{
    
}

#pragma mark -
#pragma mark Shoot Bubble
void Field::shootBubble()
{
    Vec2 calcPoint = Vec2(_lastPoint.x - _shotPoint.x, _lastPoint.y - _shotPoint.y);
    Vec2 shootDirection = calcPoint.getNormalized();
    if (bubbleToShoot != nullptr) {
        bubbleToShoot->setDirection(shootDirection);
        bubbleToShoot->setState(BubbleSprite::BubbleState::StateMoving);
        _shooting = true;
    }
}

void Field::prepareBubbleToShoot()
{
    // Create random bubble and add to layer
    int nextType = chooseNextBubble();
    bubbleToShoot = BubbleSprite::createWithSpriteFrameName(blocklist[nextType]);
    bubbleToShoot->setState(BubbleSprite::BubbleState::StateDisable);
    bubbleToShoot->setTypeBubble(nextType);
    bubbleToShoot->setScale(_block_scale_width);
    bubbleToShoot->setPosition(_shotPoint);
    this->addChild(bubbleToShoot);
}

void Field::prepareNextBubbleToShoot()
{
    
}

#pragma mark -
#pragma mark Device Event

void Field::enableTouchEvent(bool enabled)
{
    if (this->_touchListener != nullptr) {
        Director::getInstance()->getEventDispatcher()->removeEventListener(this->_touchListener);
        this->_touchListener = nullptr;
    }
    if (enabled) {
        this->_touchListener = EventListenerTouchOneByOne::create();
        _touchListener->onTouchBegan = CC_CALLBACK_2(Field::onTouchBegan, this);
        _touchListener->onTouchMoved = CC_CALLBACK_2(Field::onTouchMoved, this);
        _touchListener->onTouchEnded = CC_CALLBACK_2(Field::onTouchEnded, this);
        _touchListener->onTouchCancelled = CC_CALLBACK_2(Field::onTouchCancelled, this);
        Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(_touchListener, this);
    }
}

#pragma mark -

#pragma mark Collisions
void Field::checkWallCollisions()
{
    //    BubbleSprite* bubbleToShoot = _ShotBubble[0];
    // Check just if there is a shooting bubble and it's actually moving
    if (bubbleToShoot != nullptr && bubbleToShoot->getState() == BubbleSprite::BubbleState::StateMoving) {
        if (bubbleToShoot->getPositionX() < HORIZONTAL_OFFSET) {
            // Bounced in left wall
            Vec2 u = bubbleToShoot->getDirection();
            
            u = Vec2(-u.x, u.y);
            bubbleToShoot->setDirection(u);
            
        } else if(bubbleToShoot->getPositionX() > _winSize.width - HORIZONTAL_OFFSET){
            // Bounced in right wall
            Vec2 u = bubbleToShoot->getDirection();
            
            u = Vec2(-u.x, u.y);
            bubbleToShoot->setDirection(u);
        }
        if (bubbleToShoot->getPositionY() > _winSize.height + _scaled_height) {
            bubbleToShoot->removeFromParentAndCleanup(true);
            prepareBubbleToShoot();
            _shooting = false;
        }
    }
}

BubbleSprite* Field::checkBubblesCollisions()
{
    log("checkBubblesCollisions start");
    // Check just if there is a shooting bubble and it's actually moving
    if (bubbleToShoot != nullptr && bubbleToShoot->getState() == BubbleSprite::BubbleState::StateMoving) {
        
        // Loop through all bubbles to check collision
        for (std::vector< std::vector<BubbleSprite*> >::iterator it = bubblesGrid->begin(); it != bubblesGrid->end(); it++) {
            std::vector<BubbleSprite*> rows = *it;
            for (int j = 0; j < FIELD_WIDTH; j++) {
                BubbleSprite* bubble = rows.at(j);
                if (bubble != nullptr) {
                    float distance = bubbleToShoot->getPosition().getDistance(bubble->getPosition());
                    if (distance < _scaled_width * 0.9f) {
                        bubbleToShoot->setState(BubbleSprite::BubbleState::StateStatic);
                        _shooting = false;
                        log("checkBubblesCollisions end state true");
                        return bubble;
                    }
                }
            }
        }
    }
    
    log("checkBubblesCollisions end state false");
    return nullptr;
}
#pragma mark -

int Field::chooseNextBubble()
{
    bool enable[BLOCK_PATTERN_MAX];
    for (int b = 0; b<BLOCK_PATTERN_MAX; b++) {
        enable[b] = false;
    }
    // フィールドに登録されているバブルの種類数を算出
    int n = 0;
    for (std::vector< std::vector<BubbleSprite*> >::iterator it = bubblesGrid->begin(); it < bubblesGrid->end(); it++) {
        std::vector<BubbleSprite*> rows = *it;
        for (int j = 0; j < FIELD_WIDTH; j++) {
            BubbleSprite* bubble = rows.at(j);
            if (bubble != nullptr) {
                int type = bubble->getTypeBubble();
                if (type < BLOCK_PATTERN_MAX && !enable[type]) {
                    enable[type] = true;
                    ++n;
                }
            }
            //            log("(%d, %d) = %p", i, j, rows.at(j));
        }
    }
    // フィールドに登録されているバブルの種類数分の抽選値を算出
    int r = arc4random_uniform(n);
    for (int i = 0; i < BLOCK_PATTERN_MAX - 1; ++i) {
        if (enable[i]) {
            if (--r < 0) {
                return i + 1;
            }
        }
    }
    // フィールドに登録されているバブルの抽選に漏れた場合はバブルの中から無作為に抽選
    return arc4random_uniform(BLOCK_PATTERN_MAX-1);
}

void Field::updateNextBubble()
{
}
