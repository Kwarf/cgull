#pragma once

#ifndef NDEBUG

#include <functional>
#include <string>

#include <CoreServices/CoreServices.h>

class FileWatcher {
public:
    FileWatcher(const std::string &path, std::function<void(void *, const std::string &)> callback,
                void *callbackContext = nullptr)
        : callback(callback), context(callbackContext) {
        FSEventStreamContext context = {0};
        context.info = this;

        CFStringRef pathRef = CFStringCreateWithCString(kCFAllocatorDefault, path.c_str(), kCFStringEncodingUTF8);
        CFArrayRef paths = CFArrayCreate(NULL, (const void **)&pathRef, 1, NULL);
        stream = FSEventStreamCreate(kCFAllocatorDefault, (FSEventStreamCallback)event_callback, &context, paths,
                                     kFSEventStreamEventIdSinceNow, 1.0, kFSEventStreamCreateFlagFileEvents);
        FSEventStreamSetDispatchQueue(stream, dispatch_get_main_queue());
        FSEventStreamStart(stream);
    }

private:
    static void event_callback(ConstFSEventStreamRef streamRef, void *clientCallBackInfo, size_t numEvents,
                               void *eventPaths, const FSEventStreamEventFlags eventFlags[],
                               const FSEventStreamEventId eventIds[]) {
        FileWatcher *watcher = (FileWatcher *)clientCallBackInfo;
        char **paths = static_cast<char **>(eventPaths);
        for (int i = 0; i < numEvents; ++i) {
            if ((eventFlags[i] & kFSEventStreamEventFlagItemModified) == 0) {
                continue;
            }
            watcher->callback(watcher->context, paths[i]);
        }
    }

    FSEventStreamRef stream;
    std::function<void(void *, const std::string &)> callback;
    void *context;
};

#endif // NDEBUG
