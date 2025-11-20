//
// Created by ClapEcho233 on 2025/11/18.
//

#ifndef CELESTE_RENDER_H
#define CELESTE_RENDER_H

#include <SFML/Graphics.hpp>

/**
 * @brief 基于 Viewport 的固定比例渲染器类
 *
 * 这个类使用 SFML 的 Viewport 系统来实现固定比例渲染，通过调整视口范围来保持固定的宽高比，自动处理黑边
 */
class ViewportAspectRenderer {
private:
    sf::View gameView;              /**< 游戏世界视图，用于相机控制 */
    sf::Vector2f baseViewSize;      /**< 基础视图尺寸 */
    float targetAspectRatio;        /**< 目标宽高比 */
    sf::Color borderColor;          /**< 黑边颜色 */

public:
    /**
     * @brief 构造函数
     * @param width 基础视图宽度
     * @param height 基础视图高度
     * @param borderColor 黑边颜色
     */
    ViewportAspectRenderer(float width = 1920.0f, float height = 1104.0f,
                          const sf::Color& borderColor = sf::Color::Black);

    /**
     * @brief 处理窗口大小变化事件，更新视口设置
     * @param window 目标窗口引用
     */
    void handleResize(const sf::RenderWindow& window);

    /**
     * @brief 处理窗口大小变化事件，更新视口设置
     * @param windowSize 新的窗口大小
     */
    void handleResize(const sf::Vector2u& windowSize);

    /**
     * @brief 应用视图到目标窗口
     * @param window 目标窗口引用
     */
    void applyView(sf::RenderWindow& window) const;

    /**
     * @brief 获取游戏世界视图（可修改）
     * @return 游戏世界视图的引用
     */
    sf::View& getView();

    /**
     * @brief 获取游戏世界视图（只读）
     * @return 游戏世界视图的常量引用
     */
    const sf::View& getView() const;

    /**
     * @brief 设置游戏世界视图
     * @param view 新的视图
     */
    void setView(const sf::View& view);

    /**
     * @brief 重置视图到默认状态
     */
    void resetView();

    /**
     * @brief 获取基础视图尺寸
     * @return 基础视图尺寸
     */
    sf::Vector2f getBaseViewSize() const;

    /**
     * @brief 设置基础视图尺寸
     * @param width 新的基础宽度
     * @param height 新的基础高度
     */
    void setBaseViewSize(float width, float height);

    /**
     * @brief 设置基础视图尺寸
     * @param size 新的基础尺寸
     */
    void setBaseViewSize(const sf::Vector2f& size);

    /**
     * @brief 获取目标宽高比
     * @return 目标宽高比
     */
    float getAspectRatio() const;

    /**
     * @brief 设置黑边颜色
     * @param color 新的黑边颜色
     */
    void setBorderColor(const sf::Color& color);

    /**
     * @brief 获取当前黑边颜色
     * @return 当前黑边颜色
     */
    sf::Color getBorderColor() const;

    /**
     * @brief 设置相机中心位置
     * @param x X坐标
     * @param y Y坐标
     */
    void setCameraCenter(float x, float y);

    /**
     * @brief 设置相机中心位置
     * @param center 中心点坐标
     */
    void setCameraCenter(const sf::Vector2f& center);

    /**
     * @brief 移动相机
     * @param offsetX X轴偏移量
     * @param offsetY Y轴偏移量
     */
    void moveCamera(float offsetX, float offsetY);

    /**
     * @brief 移动相机
     * @param offset 偏移量向量
     */
    void moveCamera(const sf::Vector2f& offset);

    /**
     * @brief 缩放相机
     * @param factor 缩放因子（大于1缩小，小于1放大）
     */
    void zoomCamera(float factor);

    /**
     * @brief 设置相机视图大小
     * @param width 视图宽度
     * @param height 视图高度
     */
    void setCameraSize(float width, float height);

    /**
     * @brief 设置相机视图大小
     * @param size 视图大小向量
     */
    void setCameraSize(const sf::Vector2f& size);

    /**
     * @brief 旋转相机
     * @param angle 旋转角度（度）
     */
    void rotateCamera(float angle);

    /**
     * @brief 检查点是否在当前可见的游戏世界区域内
     * @param point 要检查的点
     * @return 如果点在可见区域内返回true，否则返回false
     */
    bool isPointVisible(const sf::Vector2f& point) const;

    /**
     * @brief 获取当前可见的游戏世界区域
     * @return 表示可见区域的浮点矩形
     */
    sf::FloatRect getVisibleArea() const;
};

#endif //CELESTE_RENDER_H