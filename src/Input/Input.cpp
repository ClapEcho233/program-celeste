//
// Created by ClapEcho233 on 2025/11/21.
//

#include "Input.h"
#include <iostream>

VirtualButton::VirtualButton(Button b) :
    bufferCounter_(0),
    currentState_(false),
    previousState_(false),
    button(b),
    buffered(false),
    check(false),
    pressed(false),
    released(false) {

}

void VirtualButton::update() {
    // 保存上一帧状态
    previousState_ = currentState_;

    // 计算当前状态
    currentState_ = false;
    if (sf::Keyboard::isKeyPressed(KeyMap.at(button))) {
        currentState_ = true;
    }

    // 更新状态
    pressed = currentState_ && !previousState_;
    released = !currentState_ && previousState_;
    check = currentState_;

    // 更新输入缓冲
    if (pressed) {
        bufferCounter_ = bufferTime;
        buffered = true;
    } else if (bufferCounter_ > 0) {
        bufferCounter_ -= deltaTime;
        if (bufferCounter_ <= 0) {
            buffered = false;
        }
    }
}

void VirtualButton::ConsumeBuffer() {
    // 将当前 buffered 状态标记为已消费
    bufferCounter_ = 0;
    buffered = false;
}

void VirtualButton::ConsumePress() {
    // 将当前 pressed 状态标记为已消费
    pressed = false;
}

VirtualLR::VirtualLR() :
    left(Button::Left),
    right(Button::Right),
    value(0) {

}

void VirtualLR::update() {
    left.update();
    right.update();

    // 更新 value
    if(left.check && right.check) {
        if (left.pressed) {
            value = -1;
        } else if (right.pressed) {
            value = 1;
        }
    } else {
        if(left.check) {
            value = -1;
        } else if (right.check) {
            value = 1;
        } else {
            value = 0;
        }
    }
}

