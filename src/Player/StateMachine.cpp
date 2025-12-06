//
// Created by ClapEcho233 on 2025/12/4.
//

#include "StateMachine.h"

StateMachine::StateMachine():
    currentState_(PlayerState::Normal),
    previousState_(PlayerState::Normal),
    isInCoroutine_(false),
    coroutineTimer_(0.0),
    coroutineWaitTime_(0.0) {
    // 初始化所有回调为空
    for (int i = 0; i < static_cast<int>(PlayerState::Count); ++i) {
        updateCallbacks[i] = nullptr;
        coroutineCallbacks[i] = nullptr;
        beginCallbacks[i] = nullptr;
        endCallbacks[i] = nullptr;
    }
}

void StateMachine::setCallbacks(PlayerState state,
                               StateUpdateFunc update,
                               StateCoroutineFunc coroutine,
                               StateBeginFunc begin,
                               StateEndFunc end) {
    int index = static_cast<int>(state);
    updateCallbacks[index] = update;
    coroutineCallbacks[index] = coroutine;
    beginCallbacks[index] = begin;
    endCallbacks[index] = end;
}

void StateMachine::changeState(PlayerState newState) {
    if (currentState_ == newState) return;

    // 调用旧状态的结束回调
    int oldIndex = static_cast<int>(currentState_);
    if (endCallbacks[oldIndex]) {
        endCallbacks[oldIndex]();
    }

    // 记录状态变更
    previousState_ = currentState_;
    currentState_ = newState;

    // 重置协程状态
    isInCoroutine_ = false;
    coroutineTimer_ = 0.0f;

    // 调用新状态的开始回调
    int newIndex = static_cast<int>(newState);
    if (beginCallbacks[newIndex]) {
        beginCallbacks[newIndex]();
    }

    // 如果新状态有协程，启动它
    startCoroutine();

    std::cout << "State changed from " << static_cast<int>(previousState_)
              << " to " << static_cast<int>(currentState_) << std::endl;
}

void StateMachine::startCoroutine() {
    int index = static_cast<int>(currentState_);
    if (coroutineCallbacks[index]) {
        isInCoroutine_ = true;
        coroutineTimer_ = 0.0;
        coroutineWaitTime_ = 0.0;
    }
}

void StateMachine::updateCoroutine() {
    // if (!isInCoroutine_) return;

    int index = static_cast<int>(currentState_);
    if (!coroutineCallbacks[index]) {
        isInCoroutine_ = false;
        return;
    }

    // 检查是否需要等待
    if (coroutineTimer_ < coroutineWaitTime_) {
        coroutineTimer_ += deltaTime;
        return;
    }

    // 执行协程的一步
    CoroutineResult result = coroutineCallbacks[index]();

    if (result.isDone) {
        // 协程完成
        isInCoroutine_ = false;
    } else {
        // 设置下一次等待时间
        coroutineWaitTime_ = result.waitTime;
        coroutineTimer_ = 0.0f;
    }
}

void StateMachine::update() {
    // 如果正在执行协程，先更新协程
    if (isInCoroutine_) {
        updateCoroutine();
        return;
    }

    // 否则执行正常的状态更新
    int index = static_cast<int>(currentState_);
    if (updateCallbacks[index]) {
        PlayerState nextState = updateCallbacks[index]();

        // 如果状态改变，进行切换
        if (nextState != currentState_) {
            changeState(nextState);
        }
    }
}