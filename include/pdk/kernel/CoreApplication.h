// @copyright 2017-2018 zzu_softboy <zzu_softboy@163.com>
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Created by softboy on 2018/01/23.

#ifndef PDK_KERNEL_ABSTRACT_CORE_APPLICATION_H
#define PDK_KERNEL_ABSTRACT_CORE_APPLICATION_H

#include "pdk/global/Global.h"
#include "pdk/kernel/Object.h"
#include "pdk/kernel/CoreEvent.h"
#include "pdk/kernel/EventLoop.h"
#include "pdk/utils/ScopedPointer.h"
#include "pdk/base/ds/StringList.h"
#include <string>

#define PDK_RETRIEVE_APP_INSTANCE() pdk::kernel::CoreApplication::getInstance();

namespace pdk {

namespace kernel {

// forward declare class with namespace
namespace internal {
class CoreApplicationPrivate;
class EventDispatcherUNIXPrivate;
} // internal

class PostEventList;
class AbstractEventDispatcher;
class AbstractNativeEventFilter;

using internal::CoreApplicationPrivate;
using pdk::ds::StringList;
using pdk::lang::String;

class PDK_CORE_EXPORT CoreApplication : public Object
{
public:
   enum { 
      ApplicationFlags = 0x000001// PDK_VERSION
   };
   
   CoreApplication(int &argc, char **argv, int = ApplicationFlags);
   ~CoreApplication();
   
   static StringList getArguments();
   void setOrgDomain(const String &domain);
   String getOrgDomain();
   void setOrgName(const String &name);
   String getOrgName();
   void setAppName(const String &name);
   String getAppName();
   void setAppVersion(const String &version);
   String getAppVersion();
   static void setSetuidAllowed(bool allow);
   static bool isSetuidAllowed();
   static CoreApplication *getInstance()
   {
      return sm_self;
   }
   static int exec();
   static void processEvents(EventLoop::ProcessEventsFlags flags = EventLoop::AllEvents);
   static void processEvents(EventLoop::ProcessEventsFlags flags, int maxtime);
   static void exit(int retcode=0);
   
   static bool sendEvent(Object *receiver, Event *event);
   static void postEvent(Object *receiver, Event *event, pdk::EventPriority priority = pdk::EventPriority::NormalEventPriority);
   static void sendPostedEvents(Object *receiver = nullptr, Event::Type eventType = Event::Type::None);
   static void removePostedEvents(Object *receiver, int eventType = 0);
   static AbstractEventDispatcher *eventDispatcher();
   static void setEventDispatcher(AbstractEventDispatcher *eventDispatcher);
   virtual bool notify(Object *, Event *);
   static bool startingUp();
   static bool closingDown();
   
   static String getAppDirPath();
   static String getAppFilePath();
   static pdk::pint64 getAppPid();
   
   static void setLibraryPaths(const StringList &);
   static StringList getLibraryPaths();
   static void addLibraryPath(const String &);
   static void removeLibraryPath(const String &);
   
   static void flush();
   
   void installNativeEventFilter(AbstractNativeEventFilter *filterObj);
   void removeNativeEventFilter(AbstractNativeEventFilter *filterObj);
   static bool isQuitLockEnabled();
   static void setQuitLockEnabled(bool enabled);
   static void quit();
   
   // signals
   // void aboutToQuit(PrivateSignal);
   // void organizationNameChanged();
   // void organizationDomainChanged();
   // void applicationNameChanged();
   // void applicationVersionChanged();
   
protected:
   bool event(Event *) override;
   virtual bool compressEvent(Event *, Object *receiver, PostEventList *);
   CoreApplication(CoreApplicationPrivate &p);
   
private:
   static bool sendSpontaneousEvent(Object *receiver, Event *event);
   static bool notifyInternal(Object *receiver, Event *);
   friend class internal::EventDispatcherUNIXPrivate;
   friend PDK_CORE_EXPORT String retrieve_app_name();
   friend class ClassFactory;
private:
   PDK_DISABLE_COPY(CoreApplication);
   PDK_DECLARE_PRIVATE(CoreApplication);
   
protected:
   pdk::utils::ScopedPointer<CoreApplicationPrivate> m_implPtr;
   
private:
   static CoreApplication *sm_self;
};

inline bool CoreApplication::sendEvent(Object *receiver, Event *event)
{
   if (event) {
      event->m_spont = false; 
   }
   return notifyInternal(receiver, event);
}

inline bool CoreApplication::sendSpontaneousEvent(Object *receiver, Event *event)
{
   if (event) {
      event->m_spont = true; 
   }
   return notifyInternal(receiver, event); 
}

using StartUpFunction = void (*)();
using CleanUpFunction = void (*)();

PDK_CORE_EXPORT void add_pre_routine(StartUpFunction);
PDK_CORE_EXPORT void add_post_routine(CleanUpFunction);
PDK_CORE_EXPORT void remove_post_routine(CleanUpFunction);
PDK_CORE_EXPORT String retrieve_app_name();                // get application name

#define PDK_COREAPP_STARTUP_FUNCTION(AFUNC) \
    static void AFUNC ## _ctor_function() {  \
        qAddPreRoutine(AFUNC);        \
    }                                 \
    PDK_CONSTRUCTOR_FUNCTION(AFUNC ## _ctor_function)

PDK_CORE_EXPORT uint global_posted_events_count();
void PDK_CORE_EXPORT call_post_routines();

} // kernel
} // pdk

#endif // PDK_KERNEL_ABSTRACT_CORE_APPLICATION_H
