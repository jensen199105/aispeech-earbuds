/**
 * Project  : AIWakeup.v1.0.7
 * FileName : api/api_wakeup.h
 *
 * COPYRIGHT (C) 2015-2016, AISpeech Ltd.. All rights reserved.
 */
#ifndef __AISPEECH__API_WAKEUP_H__
#define __AISPEECH__API_WAKEUP_H__

typedef enum wakeup_status
{
    WAKEUP_STATUS_ERROR = -1,     // 唤醒出现异常
    WAKEUP_STATUS_WAIT,           // 等待异常
    WAKEUP_STATUS_WOKEN,          // 已唤醒
} wakeup_status_t;

typedef enum thresh_type
{
    THRESH_TYPE_DEFAULT = 0,    // 默认, 安静环境，对应env中的thresh
    THRESH_TYPE_WIND    = 1,    // 风噪，对应env中的 thresh_wind
} thresh_type_e;

typedef struct wakeup wakeup_t;

/**
 * 回调函数定义
 *
 * @param user_data:用户传入的数据结构指针
 * @param status: 唤醒状态
 * @param json: 返回唤醒模块的信息
 * @param bytes: Json串的字节数
 */
typedef int (*wakeup_handler_t)(void *user_data, wakeup_status_t status, const char *json, int bytes);
typedef int (*vad_handler_t)(void *user_data, int frame_status, int frame_index);
typedef int (*wakeup_feature_handler_t)(void *user_data, int *f, int count);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 创建唤醒模块的实例
 *
 * @param fn: 资源文件的路径
 * @param json: 个性化定制时，加密后的唤醒配置信息，目前不用
 */
wakeup_t *wakeup_new(char *mem_ptr, unsigned int size);

/**
 * 清理唤醒实例并释放内存
 * @param w:唤醒模块的instance
 */
void wakeup_delete(wakeup_t *w);

/**
 * 重置唤醒模块
 *
 * @param w: 唤醒模块的intance
 */
void wakeup_reset(wakeup_t *w);

/**
 * 启动唤醒模块
 *
 * @param w: 唤醒模块的instance
 * @param env: 自定义参数
 * @param bytes: 自定义参数的长度
 */
int wakeup_start(wakeup_t *w, char *env, int bytes);

/**
 * 提供音频数据给唤醒模块,并判断唤醒状态
 *
 * @param w: 唤醒模块的instance
 * @param data: 音频数据
 * @param bytes: 音频数据的字节数
 *
 * @return 唤醒状态
 */
wakeup_status_t wakeup_feed(wakeup_t *w, char *data, int bytes);

/**
 * 清理唤醒模块的cache以及状态
 *
 * @param w: 唤醒模块的instance
 */
wakeup_status_t wakeup_end(wakeup_t *w);

/**
 * wakeup version
 *
 */
const char *wakeup_version(void);

/**
 * 注册状态的回调函数
 *
 * @param w: 唤醒模块的instance
 * @param user_data:用户的数据结构
 * @param func: 注册的函数指针
 */
void wakeup_register_handler(wakeup_t *w, void *user_data, wakeup_handler_t func);
void wakeup_register_feature_handler(wakeup_t *w, void *user_data, wakeup_feature_handler_t func);

#ifndef WKP_VAD_DISABLE
void wakeup_register_vad_handler(wakeup_t *w, void *user_data, vad_handler_t func);
#endif

/**
 * 设置使用的阈值类型
 *
 * @param w: 唤醒模块的instance
 * @param thresh_type: 阈值类型, 取值参考 thresh_type_e
 *
 * @return 0：正确; -1：错误
 */

int wakeup_set_thresh(wakeup_t *w, int thresh_type);

/*
 * 重置唤醒词和阈值, 该接口在早期的dnn项目中提供，存在风险，对有需要的客户开放
 *
 * @param w: 唤醒模块的instance
 * @param mem_ptr: wakeup_new时传入的堆内存首地址
 * @param func: 唤醒回调函数
 * @param env: 新的唤醒词和阈值配置
 *
 */
// void wakeup_reset_wkpwords(wakeup_t *w, unsigned char *mem_ptr, wakeup_handler_t func,char *env);


#ifdef __cplusplus
}
#endif

#endif //__AISPEECH__API_WAKEUP_H__
