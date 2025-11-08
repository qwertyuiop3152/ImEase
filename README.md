| 函数 | 常量 | 描述 |
|------|------|------|
| Linear | `i_linear` | 线性插值 |
| Ease In | `i_ease_in` | 加速进入 |
| Ease Out | `i_ease_out` | 减速退出 |
| Ease In-Out | `i_ease_in_out` | 加速进入，减速退出 |
| Elastic In | `i_elastic_in` | 弹性进入 |
| Elastic Out | `i_elastic_out` | 弹性退出 |
| Elastic In-Out | `i_elastic_in_out` | 弹性进入和退出 |
| Bounce In | `i_bounce_in` | 弹跳进入 |
| Bounce Out | `i_bounce_out` | 弹跳退出 |
| Bounce In-Out | `i_bounce_in_out` | 弹跳进入和退出 |

auto pos_anim = std::make_shared<anim_vec2>(
    vec2(0, 0),     // 初始位置
    0.5f,           // 持续时间
    i_bounce_out    // 弹跳效果
);
