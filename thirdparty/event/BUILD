cc_library(name = 'event',
    deps = [':event_core',
            ':event_extra',
            ],
    warning = 'no',
)

cc_library(name = 'event_core',
    srcs = [
    'event.c',
    'evthread.c',
    'buffer.c',
    'bufferevent.c',
    'bufferevent_sock.c',
    'bufferevent_filter.c',
    'bufferevent_pair.c',
    'listener.c',
    'bufferevent_ratelim.c',
    'evmap.c',
    'log.c',
    'evutil.c',
    'evutil_rand.c',
    'strlcpy.c',
    'select.c',
    'poll.c',
    'epoll.c',
    'signal.c',
    ],
    warning = 'no',
    deps = ['#rt',],
)

cc_library(name = 'event_extra',
    srcs = [
    'event_tagging.c',
    'http.c',
    'evdns.c',
    'evrpc.c',
    ],
    deps = [':event_core',],
    warning = 'no',
)
            
cc_library(name = 'event_pthread',
    srcs = [
    'evthread_pthread.c',
    ],
    warning = 'no',
)

