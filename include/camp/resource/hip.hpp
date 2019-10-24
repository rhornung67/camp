#ifndef __CAMP_HIP_HPP
#define __CAMP_HIP_HPP

#include "camp/resource/event.hpp"
#include "camp/resource/platform.hpp"

#ifdef CAMP_HAVE_HIP
#include <hip/hip_runtime.hpp>

namespace camp
{
namespace resources
{
  inline namespace v1
  {

    class HipEvent
    {
    public:
      HipEvent(hipStream_t stream)
      {
        hipEventCreateWithFlags(&m_event, hipEventDisableTiming);
        hipEventRecord(m_event, stream);
      }
      bool check() const { return (hipEventQuery(m_event) == hipSuccess); }
      void wait() const { hipEventSynchronize(m_event); }
      hipEvent_t getHipEvent_t() const { return m_event; }

    private:
      hipEvent_t m_event;
    };

    class Hip
    {
      static hipStream_t get_a_stream(int num)
      {
        static hipStream_t streams[16] = {};
        static int previous = 0;

        static std::once_flag m_onceFlag;
        static std::mutex m_mtx;

        std::call_once(m_onceFlag, [] {
          if (streams[0] == nullptr) {
            for (auto &s : streams) {
              hipStreamCreate(&s);
            }
          }
        });

        if (num < 0) {
          m_mtx.lock();
          previous = (previous + 1) % 16;
          m_mtx.unlock();
          return streams[previous];
        }

        return streams[num % 16];
      }

    public:
      Hip(int group = -1) : stream(get_a_stream(group)) {}

      // Methods
      Platform get_platform() { return Platform::hip; }
      static Hip &get_default()
      {
        static Hip h;
        return h;
      }
      HipEvent get_event() { return HipEvent(get_stream()); }
      Event get_event_erased() { return Event{HipEvent(get_stream())}; }
      void wait() { hipStreamSynchronize(stream); }
      void wait_on(Event *e)
      {
        if (e->test_get<HipEvent>()) {
          hipStreamWaitEvent(get_stream(),
                              e->get<HipEvent>().getHipEvent_t(),
                              0);
        } else {
          e->wait();
        }
      }

      // Memory
      template <typename T>
      T *allocate(size_t size)
      {
        T *ret = nullptr;
        hipMallocManaged(&ret, sizeof(T) * size);
        return ret;
      }
      void *calloc(size_t size)
      {
        void *p = allocate<char>(size);
        this->memset(p, 0, size);
        return p;
      }
      void free(void *p) { hipFree(p); }
      void memcpy(void *dst, const void *src, size_t size)
      {
        hipMemcpyAsync(dst, src, size, hipMemcpyDefault, stream);
      }
      void memset(void *p, int val, size_t size)
      {
        hipMemsetAsync(p, val, size, stream);
      }

      hipStream_t get_stream() { return stream; }

    private:
      hipStream_t stream;
    };

  }  // namespace v1
}  // namespace resources
}  // namespace camp
#endif  //#ifdef CAMP_HAVE_HIP

#endif /* __CAMP_HIP_HPP */
