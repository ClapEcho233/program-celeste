//
// Created by ClapEcho233 on 2025/11/21.
//

#ifndef CELESTE_INPUTMANAGER_H
#define CELESTE_INPUTMANAGER_H

#include <SFML/Graphics.hpp>
#include <map>
#include "../Common/Constant.h"

enum class Button {Up, Down, Left, Right, Jump, Dash, Climb};

/**
 * 按键映射表
 */
const std::map <Button, sf::Keyboard::Key> KeyMap = {
    {Button::Up, sf::Keyboard::Key::Up},
    {Button::Down, sf::Keyboard::Key::Down},
    {Button::Left, sf::Keyboard::Key::Left},
    {Button::Right, sf::Keyboard::Key::Right},
    {Button::Jump, sf::Keyboard::Key::C},
    {Button::Dash, sf::Keyboard::Key::X},
    {Button::Climb, sf::Keyboard::Key::Z}
};

class VirtualButton {
private:
    float bufferCounter_;
    bool currentState_; // 当前状态
    bool previousState_; // 上一帧状态

public:
    Button button;
    float bufferTime = 0.1; // 缓冲时间
    bool buffered; // 是否在缓冲状态
    bool check; // 当前状态
    bool pressed; // 是否按下
    bool released; // 是否抬起

    VirtualButton(Button b);

    void update ();

    bool getOriginalInput();

    /**
     * 消费 Buffer 状态
     */
    void consumeBuffer();

    /**
     * 消费 Press 状态
     */
    void consumePress();
};

class VirtualLR {
private:

public:
    VirtualButton left;
    VirtualButton right;

    int value; // 1:right -1:left

    VirtualLR();

    void update();
};

class Input {
private:

public:
    VirtualButton up;
    VirtualButton down;
    VirtualLR lr;
    VirtualButton jump;
    VirtualButton dash;
    VirtualButton climb;

    Input();

    void update();
};

#endif //CELESTE_INPUTMANAGER_H