/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NU_PLAYER_H_

#define NU_PLAYER_H_

#include <media/MediaPlayerInterface.h>
#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/NativeWindowWrapper.h>

namespace android {

struct ACodec;
struct MetaData;
struct NuPlayerDriver;

struct NuPlayer : public AHandler {
    NuPlayer();

    void setUID(uid_t uid);

    void setDriver(const wp<NuPlayerDriver> &driver);

    void setDataSource(const sp<IStreamSource> &source);

    void setDataSource(
            const char *url, const KeyedVector<String8, String8> *headers);

    void setDataSource(int fd, int64_t offset, int64_t length);

    void setVideoSurfaceTexture(const sp<ISurfaceTexture> &surfaceTexture);
    void setAudioSink(const sp<MediaPlayerBase::AudioSink> &sink);
    void start();

    void pause();
    void resume();

    // Will notify the driver through "notifyResetComplete" once finished.
    void resetAsync();

    // Will notify the driver through "notifySeekComplete" once finished.
    void seekToAsync(int64_t seekTimeUs);

    status_t setVideoScalingMode(int32_t mode);

#ifndef ANDROID_DEFAULT_CODE
    void prepareAsync();
    void stop();
    enum DataSourceType {
            SOURCE_Default,
            SOURCE_HttpLive,
            SOURCE_Local,
            SOURCE_Rtsp
    };
    static bool IsNeedFlush_WhenSeek(DataSourceType SourceType, bool *needShutdown);
    // mtk80902: ALPS00448589
    sp<MetaData> getMetaData() const; 
#endif

protected:
    virtual ~NuPlayer();

    virtual void onMessageReceived(const sp<AMessage> &msg);

public:
    struct NuPlayerStreamListener;
    struct Source;


private:
    struct Decoder;
    struct GenericSource;
    struct HTTPLiveSource;
    struct Renderer;
    struct RTSPSource;
    struct StreamingSource;

    enum {
        kWhatSetDataSource              = '=DaS',
        kWhatSetVideoNativeWindow       = '=NaW',
        kWhatSetAudioSink               = '=AuS',
        kWhatMoreDataQueued             = 'more',
        kWhatStart                      = 'strt',
        kWhatScanSources                = 'scan',
        kWhatVideoNotify                = 'vidN',
        kWhatAudioNotify                = 'audN',
        kWhatRendererNotify             = 'renN',
        kWhatReset                      = 'rset',
        kWhatSeek                       = 'seek',
        kWhatPause                      = 'paus',
        kWhatResume                     = 'rsme',
#ifndef ANDROID_DEFAULT_CODE
        kWhatPrepare                    = 'prep',
        kWhatSourceNotify               = 'srcN',
	kWhatStop			= 'stop'
#endif
    };

    wp<NuPlayerDriver> mDriver;
    bool mUIDValid;
    uid_t mUID;
    sp<Source> mSource;
    sp<NativeWindowWrapper> mNativeWindow;
    sp<MediaPlayerBase::AudioSink> mAudioSink;
    sp<Decoder> mVideoDecoder;
    bool mVideoIsAVC;
    sp<Decoder> mAudioDecoder;
    sp<Renderer> mRenderer;

    bool mAudioEOS;
    bool mVideoEOS;
#ifndef ANDROID_DEFAULT_CODE
    int64_t  mVideoFirstRenderTimestamp;
#endif

    bool mScanSourcesPending;
    int32_t mScanSourcesGeneration;

    enum FlushStatus {
        NONE,
        AWAITING_DISCONTINUITY,
        FLUSHING_DECODER,
        FLUSHING_DECODER_SHUTDOWN,
        SHUTTING_DOWN_DECODER,
        FLUSHED,
        SHUT_DOWN,
    };

    // Once the current flush is complete this indicates whether the
    // notion of time has changed.
    bool mTimeDiscontinuityPending;

    FlushStatus mFlushingAudio;
    FlushStatus mFlushingVideo;
    bool mResetInProgress;
    bool mResetPostponed;

    int64_t mSkipRenderingAudioUntilMediaTimeUs;
    int64_t mSkipRenderingVideoUntilMediaTimeUs;

    int64_t mVideoLateByUs;
    int64_t mNumFramesTotal, mNumFramesDropped;

    int32_t mVideoScalingMode;

#ifndef ANDROID_DEFAULT_CODE
    int64_t mSeekTimeUs;
    mutable Mutex mLock;
    int32_t mVideoWidth;
    int32_t mVideoHeight;
    bool isSeeking_l(){return mSeekTimeUs != -1;};
    enum PrepareState {
        UNPREPARED,
        PREPARING,
        PREPARED,
        PREPARE_CANCELED
    };

    DataSourceType mDataSourceType;
    enum PlayState {
        STOPPED,
	PLAYSENDING,
        PLAYING,
	PAUSING,
        PAUSED
    };
    PrepareState mPrepare;
    PlayState mPlayState;
    bool onScanSources();
    void finishPrepare(bool bSuccess, int err = UNKNOWN_ERROR);
    void onSourceNotified(const sp<AMessage> &msg);
    bool flushAfterSeekIfNecessary();

// mtk80902: porting from AwesomePlayer, for ALPS00436540 now
// may fullfill later.
    enum {
        CACHE_UNDERRUN      = 0x80,
    };
    uint32_t mFlags;
    
#endif


    status_t instantiateDecoder(bool audio, sp<Decoder> *decoder);

    status_t feedDecoderInputData(bool audio, const sp<AMessage> &msg);
    void renderBuffer(bool audio, const sp<AMessage> &msg);

    void notifyListener(int msg, int ext1, int ext2);

    void finishFlushIfPossible();

    void flushDecoder(bool audio, bool needShutdown);

    static bool IsFlushingState(FlushStatus state, bool *needShutdown = NULL);

    void finishReset();
    void postScanSources();

    DISALLOW_EVIL_CONSTRUCTORS(NuPlayer);
};

}  // namespace android

#endif  // NU_PLAYER_H_
