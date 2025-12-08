//
// Created by ClapEcho233 on 2025/12/4.
//

#ifndef CELESTE_STATEMACHINE_H
#define CELESTE_STATEMACHINE_H

#include "../Common/Constant.h"
#include <functional>
#include <memory>
#include <iostream>
#include <libcopp/coroutine/coroutine_context_container.h>

enum class PlayerState {
    Normal = 0,
    Climb,
    Dash,
    Count // 状态总数
};

inline CoroutineMessage messages_;

// 状态回调函数类型
using StateBeginFunc = std::function<void()>;
using StateUpdateFunc = std::function<PlayerState()>;
using StateCoroutineFunc = std::function<int(void*)>;
using StateEndFunc = std::function<void()>;

class StateMachine {
private:
    PlayerState currentState_;
    PlayerState previousState_;

    // 回调函数存储
    StateUpdateFunc updateCallbacks[static_cast<int>(PlayerState::Count)];
    StateCoroutineFunc coroutineCallbacks[static_cast<int>(PlayerState::Count)];
    StateCoroutineFunc coroutineCallbacksBackup[static_cast<int>(PlayerState::Count)]; // 备份被 move 走的函数指针
    StateBeginFunc beginCallbacks[static_cast<int>(PlayerState::Count)];
    StateEndFunc endCallbacks[static_cast<int>(PlayerState::Count)];

    // 协程相关
    copp::coroutine_context_default::ptr_type co_;
    bool isInCoroutine_;
    float coroutineTimer_;
    float coroutineWaitTime_;

    void startCoroutine();
    void updateCoroutine();

public:
    StateMachine();

    // 设置状态回调
    void setCallbacks(PlayerState state,
                     StateUpdateFunc update,
                     StateCoroutineFunc coroutine = nullptr,
                     StateBeginFunc begin = nullptr,
                     StateEndFunc end = nullptr);

    // 状态管理
    void changeState(PlayerState newState);
    void update();

    // 状态查询
    PlayerState getCurrentState() const { return currentState_; }
    bool isInState(PlayerState state) const { return currentState_ == state; }
};

#endif //CELESTE_STATEMACHINE_H