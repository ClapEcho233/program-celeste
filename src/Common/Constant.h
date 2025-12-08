//
// Created by ClapEcho233 on 2025/11/21.
//

#ifndef CELESTE_CONSTANT_H
#define CELESTE_CONSTANT_H

extern float deltaTime;

// 冲刺协程返回类型
struct CoroutineMessage {
    bool isDone = false;
    float waitTime = 0.0;  // 需要等待的时间
};

#endif //CELESTE_CONSTANT_H