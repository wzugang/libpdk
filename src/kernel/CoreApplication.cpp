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
// Created by softboy on 2018/01/26.

#include "pdk/global/GlobalStatic.h"
#include "pdk/global/internal/HooksPrivate.h"
#include "pdk/kernel/CoreApplication.h"
#include "pdk/kernel/internal/CoreApplicationPrivate.h"
#include "pdk/kernel/AbstractEventDispatcher.h"
#include "pdk/kernel/CoreEvent.h"
#include "pdk/kernel/EventLoop.h"
#include "pdk/utils/ScopedPointer.h"
#include "pdk/base/os/thread/Thread.h"
#include "pdk/base/os/thread/internal/ThreadPrivate.h"
#include "pdk/base/os/thread/ThreadStorage.h"

#include <cstdlib>
#include <list>
#include <mutex>
#include <string>
#include <cstring>

#ifdef PDK_OS_UNIX
#  include <locale.h>
#  include <unistd.h>
#  include <sys/types.h>
#endif

#include <algorithm>

#if defined(PDK_OS_UNIX)
//# if defined(PDK_OS_DARWIN)
//#  include "pdk/kernel/EventDispatcherCf.h"
//# else
//#  if !defined(PDK_NO_GLIB)
//#   include "pdk/kernel/EventDispatcherGlib.h"
//#  endif
//# endif
# include "pdk/kernel/EventDispatcherUnix.h"
#endif

namespace pdk {
namespace kernel {

using pdk::utils::ScopedPointer;
using pdk::os::thread::Thread;
using pdk::os::thread::internal::ThreadData;
using pdk::os::thread::internal::PostEvent;
using pdk::os::thread::ThreadStorageData;
using pdk::kernel::internal::CoreApplicationPrivate;

extern "C" void PDK_CORE_EXPORT startup_hook()
{
}

using StartupFuncList = std::list<StartUpFunction>;
PDK_GLOBAL_STATIC(StartupFuncList, sg_preRList);
using ShutdownFuncList = std::list<CleanUpFunction>;
PDK_GLOBAL_STATIC(ShutdownFuncList, sg_postRList);

static std::mutex sg_preRoutinesMutex;

namespace {

void call_pre_routines()
{
   StartupFuncList *list = sg_preRList();
   if (!list) {
      return;
   }
   std::lock_guard<std::mutex> locker(sg_preRoutinesMutex);
   // Unlike pdk::kernel::call_post_routines, we don't empty the list, because
   // PDK_COREAPP_STARTUP_FUNCTION is a macro, so the user expects
   // the function to be executed every time CoreApplication is created.
   auto iter = list->cbegin();
   auto end = list->cend();
   while (iter != end) {
      (*iter)();
      ++iter;
   }
}

#if defined(PDK_OS_WIN)
// Check whether the command line arguments match those passed to main()
// by comparing to the global __argv/__argc (MS extension).
// Deep comparison is required since argv/argc is rebuilt by WinMain for
// GUI apps or when using MinGW due to its globbing.
inline bool is_argv_modified(int argc, char **argv)
{
   if (__argc != argc || !__argv /* wmain() */)
      return true;
   if (__argv == argv)
      return false;
   for (int a = 0; a < argc; ++a) {
      if (argv[a] != __argv[a] && strcmp(argv[a], __argv[a]))
         return true;
   }
   return false;
}

inline bool contains(int argc, char **argv, const char *needle)
{
   for (int a = 0; a < argc; ++a) {
      if (!strcmp(argv[a], needle)) {
         return true;
      }
   }
   return false;
}
#endif // PDK_OS_WIN

bool do_notify(Object *, Event *)
{
   
}
}

namespace internal {

bool CoreApplicationPrivate::sm_setuidAllowed = false;

String CoreApplicationPrivate::getAppName() const
{
//   String appName;
//   if (m_argv[0]) {
//      char *p = std::strrchr(m_argv[0], '/');
//      appName = p ? (p + 1) : m_argv[0];
//   }
//   return appName;
}

String *CoreApplicationPrivate::m_cachedAppFilePath = nullptr;

bool CoreApplicationPrivate::checkInstance(const char *method)
{
   bool flag = (CoreApplication::sm_self != nullptr);
   if (!flag) {
      // qWarning("CoreApplication::%s: Please instantiate the CoreApplication object first", method);
   }
   return flag;
}

void CoreApplicationPrivate::processCommandLineArguments()
{
   int j = m_argc ? 1 : 0;
   for (int i = 1; i < m_argc; ++i) {
      if (!m_argv[i]) {
         continue;
      }
      if (*m_argv[i] != '-') {
         m_argv[j++] = m_argv[i];
         continue;
      }
      const char *arg = m_argv[i];
      if (arg[1] == '-') { // startsWith("--")
         ++arg;
      }
      m_argv[j++] = m_argv[i];
   }
   
   if (j < m_argc) {
      m_argv[j] = 0;
      m_argc = j;
   }
}

} // anonymous namespace

void add_pre_routine(StartUpFunction func)
{
   StartupFuncList *list = sg_preRList();
   if (!list) {
      return;
   }
   // Due to C++11 parallel dynamic initialization, this can be called
   // from multiple threads.
   std::lock_guard<std::mutex> locker(sg_preRoutinesMutex);
   if (CoreApplication::getInstance()) {
      func();
   }
   list->push_back(func);
}

void add_post_routine(CleanUpFunction func)
{
   ShutdownFuncList *list = sg_postRList();
   if (!list) {
      return;
   }
   list->push_front(func);
}

void remove_post_routine(CleanUpFunction func)
{
   ShutdownFuncList *list = sg_postRList();
   if (!list) {
      return;
   }
   list->remove(func);
}

void call_post_routines()
{
   ShutdownFuncList *list = nullptr;
   try {
      list = sg_postRList();
   } catch (const std::bad_alloc &) {
      // ignore - if we can't allocate a post routine list,
      // there's a high probability that there's no post
      // routine to be executed :)
   }
   if (!list) {
      return;
   }
   while (!list->empty()) {
      (list->front())();
      list->pop_front();
   }
}

static bool sg_localeInitialized = false;

#ifdef PDK_OS_UNIX
pdk::HANDLE pdk_application_thread_id = 0;
#endif

uint global_posted_events_count()
{
   ThreadData *currentThreadData = ThreadData::current();
   return currentThreadData->m_postEventList.size() - currentThreadData->m_postEventList.m_startOffset;
}

CoreApplication *CoreApplication::sm_self = nullptr;
struct CoreApplicationData {
   CoreApplicationData() noexcept
   {
      m_appNameSet = false;
   }
   ~CoreApplicationData()
   {
      // cleanup the QAdoptedThread created for the main() thread
      if (CoreApplicationPrivate::sm_theMainThread) {
         internal::ThreadData *data = internal::ThreadData::get(CoreApplicationPrivate::sm_theMainThread);
         data->deref(); // deletes the data and the adopted thread
      }
   }
   
   String m_orgName;
   String m_orgDomain;
   String m_appName; // application name, initially from argv[0], can then be modified.
   String m_appVersion;
   bool m_appNameSet; // true if setApplicationName was called
   
#ifndef PDK_NO_LIBRARY
   ScopedPointer<StringList> m_appLibpaths;
   ScopedPointer<StringList> m_manualLibpaths;
#endif
};

PDK_GLOBAL_STATIC(CoreApplicationData, sg_coreAppData);
static bool sg_quitLockRefEnabled = true;

namespace internal {
bool CoreApplicationPrivate::sm_isAppRunning = false;
bool CoreApplicationPrivate::sm_isAppClosing = false;
AbstractEventDispatcher *CoreApplicationPrivate::sm_eventDispatcher = nullptr;

CoreApplicationPrivate::CoreApplicationPrivate(int &aargc, char **aargv, uint flags)
   : ObjectPrivate(),
     #if defined(PDK_OS_WIN)
     m_origArgc(0),
     m_origArgv(Q_NULLPTR),
     #endif
     m_applicationType(CoreApplicationPrivate::Type::Tty),
     m_inExec(false),
     m_aboutToQuitEmitted(false),
     m_threadDataClean(false),
     m_argc(aargc),
     m_argv(aargv)
{
}

CoreApplicationPrivate::~CoreApplicationPrivate()
{
   cleanupThreadData();
#if defined(PDK_OS_WIN)
   delete [] m_origArgv;
#endif
   CoreApplicationPrivate::clearAppFilePath();
}

void CoreApplicationPrivate::cleanupThreadData()
{
   if (m_threadData && !m_threadDataClean) {
      void *data = &m_threadData->m_tls;
      ThreadStorageData::finish(reinterpret_cast<void **>(data));
   }
   std::lock_guard<std::mutex> locker(m_threadData->m_postEventList.m_mutex);
   for (size_t i = 0; i < m_threadData->m_postEventList.size(); ++i) {
      const PostEvent &pe = m_threadData->m_postEventList.at(i);
      if (pe.m_event) {
         --pe.m_receiver->getImplPtr()->m_postedEvents;
         pe.m_event->m_posted = false;
         delete pe.m_event;
      }
   }
   m_threadData->m_postEventList.clear();
   m_threadData->m_postEventList.m_recursion = 0;
   m_threadData->m_quitNow = false;
   m_threadDataClean = true;
}

void CoreApplicationPrivate::createEventDispatcher()
{
   PDK_Q(CoreApplication);
#if defined(PDK_OS_UNIX)
#  if defined(PDK_OS_DARWIN)
   bool ok = false;
   // int value = environment_variable_int_value("PDK_EVENT_DISPATCHER_CORE_FOUNDATION", &ok);
   int value = 0;
   if (ok && value > 0) {
      // m_eventDispatcher = new QEventDispatcherCoreFoundation(apiPtr);
   } else {
      sm_eventDispatcher = new EventDispatcherUNIX(apiPtr);
   }
#  endif
#else
#  error "pdk::kernel::EventDispatcher not yet ported to this platform"
#endif
}

void CoreApplicationPrivate::eventDispatcherReady()
{
}

BasicAtomicPointer<Thread> CoreApplicationPrivate::sm_theMainThread = PDK_BASIC_ATOMIC_INITIALIZER(0);

Thread *CoreApplicationPrivate::getMainThread()
{
   PDK_ASSERT(sm_theMainThread.load() != nullptr);
   return sm_theMainThread.load();
}

bool CoreApplicationPrivate::threadRequiresCoreApplication()
{
   ThreadData *data = ThreadData::current(false);
   if (!data) {
      return true;
   }
   return data->m_requiresCoreApplication;
}

void CoreApplicationPrivate::checkReceiverThread(Object *receiver)
{
   Thread *currentThread = Thread::getCurrentThread();
   Thread *thread = receiver->getThread();
   //   PDK_ASSERT_X(currentThread == thr || !thr,
   //              "CoreApplication::sendEvent",
   //              QString::fromLatin1("Cannot send events to objects owned by a different thread. "
   //                                  "Current thread %1. Receiver '%2' (of type '%3') was created in thread %4")
   //              .arg(QString::number((quintptr) currentThread, 16))
   //              .arg(receiver->objectName())
   //              .arg(QLatin1String(receiver->metaObject()->className()))
   //              .arg(QString::number((quintptr) thr, 16))
   //              .toLocal8Bit().data());
   PDK_ASSERT_X(currentThread == thread || !thread,
                "CoreApplication::sendEvent",
                "Cannot send events to objects owned by a different thread.");
   PDK_UNUSED(currentThread);
   PDK_UNUSED(thread);
}

void CoreApplicationPrivate::appendAppPathToLibPaths()
{
//#ifndef PDK_NO_LIBRARY
//   StringList *appLibPaths = sg_coreAppData()->m_appLibpaths.getData();
//   if (!appLibPaths) {
//      sg_coreAppData->m_appLibpaths.reset(appLibPaths = new StringList);
//   }
//   String appLocation = CoreApplication::getAppFilePath();
//   appLocation.erase(appLocation.find_last_of('/'));
//   // TODO we need check exist
//   auto iter = std::find(appLibPaths->cbegin(), appLibPaths->cend(), appLocation);
//   if(iter == appLibPaths->cend()) {
//      appLibPaths->push_back(appLocation);
//   }
//#endif
}

void CoreApplicationPrivate::initLocale()
{
   if (sg_localeInitialized) {
      return;
   }
   sg_localeInitialized = true;
#ifdef PDK_OS_UNIX
   setlocale(LC_ALL, "");
#endif
}

void CoreApplicationPrivate::init()
{
}

bool CoreApplicationPrivate::sendThroughApplicationEventFilters(Object *receiver, Event *event)
{
}

bool CoreApplicationPrivate::sendThroughObjectEventFilters(Object *receiver, Event *event)
{
   
}

bool CoreApplicationPrivate::notifyHelper(Object *receiver, Event *event)
{
}

// Cleanup after eventLoop is done executing in CoreApplication::exec().
// This is for use cases in which CoreApplication is instantiated by a
// library and not by an application executable, for example, Active X
// servers.

void CoreApplicationPrivate::execCleanup()
{
}


} // internal

String retrieve_app_name()
{
   if (!CoreApplicationPrivate::checkInstance("retrieve_app_name")) {
      return String();
   }
   return CoreApplication::getInstance()->getImplPtr()->getAppName();
}

CoreApplication::CoreApplication(CoreApplicationPrivate &p)
   : Object(p, 0)
{
}

CoreApplication::CoreApplication(int &argc, char **argv, int internal)
   : Object(*new CoreApplicationPrivate(argc, argv, internal))
{
   
}

void CoreApplication::flush()
{
}

CoreApplication::~CoreApplication()
{
}

void CoreApplication::setSetuidAllowed(bool allow)
{
}

bool CoreApplication::isSetuidAllowed()
{
}

bool CoreApplication::isQuitLockEnabled()
{
}

void CoreApplication::setQuitLockEnabled(bool enabled)
{
}

bool CoreApplication::notifyInternal(Object *receiver, Event *event)
{
}

bool CoreApplication::notify(Object *receiver, Event *event)
{
}

bool CoreApplication::startingUp()
{
}

bool CoreApplication::closingDown()
{
}

void CoreApplication::processEvents(EventLoop::ProcessEventsFlags flags)
{
}

void CoreApplication::processEvents(EventLoop::ProcessEventsFlags flags, int maxtime)
{
}

int CoreApplication::exec()
{
   
}

void CoreApplication::exit(int returnCode)
{
}

void CoreApplication::postEvent(Object *receiver, Event *event, pdk::EventPriority priority)
{
}

bool CoreApplication::compressEvent(Event *event, Object *receiver, PostEventList *postedEvents)
{
}

void CoreApplication::sendPostedEvents(Object *receiver, Event::Type eventType)
{
}

namespace internal {

void CoreApplicationPrivate::sendPostedEvents(Object *receiver, int eventType,
                                              ThreadData *data)
{
}

void CoreApplicationPrivate::removePostedEvent(Event * event)
{
}

void CoreApplicationPrivate::ref()
{
}

void CoreApplicationPrivate::deref()
{
}

void CoreApplicationPrivate::maybeQuit()
{
}

// Makes it possible to point QCoreApplication to a custom location to ensure
// the directory is added to the patch, and qt.conf and deployed plugins are
// found from there. This is for use cases in which QGuiApplication is
// instantiated by a library and not by an application executable, for example,
// Active X servers.

void CoreApplicationPrivate::setAppFilePath(const String &path)
{
}


}

void CoreApplication::removePostedEvents(Object *receiver, int eventType)
{
}

bool CoreApplication::event(Event *e)
{
}

void CoreApplication::quit()
{
}

String CoreApplication::getAppDirPath()
{
}

String CoreApplication::getAppFilePath()
{
}

pdk::pint64 CoreApplication::getAppPid()
{
}

StringList CoreApplication::getArguments()
{
}

void CoreApplication::setOrgName(const String &orgName)
{
}

String CoreApplication::getOrgName()
{
}

void CoreApplication::setOrgDomain(const String &orgDomain)
{
}

String CoreApplication::getOrgDomain()
{
}

void CoreApplication::setAppName(const String &application)
{
}

String CoreApplication::getAppName()
{
}

void CoreApplication::setAppVersion(const String &version)
{
}

String CoreApplication::getAppVersion()
{
   
}

PDK_GLOBAL_STATIC(std::recursive_mutex, sg_libraryPathMutex);

StringList CoreApplication::getLibraryPaths()
{
}

void CoreApplication::setLibraryPaths(const StringList &paths)
{
}

void CoreApplication::addLibraryPath(const String &path)
{
}

void CoreApplication::removeLibraryPath(const String &path)
{
}

void CoreApplication::installNativeEventFilter(AbstractNativeEventFilter *filterObj)
{
}

void CoreApplication::removeNativeEventFilter(AbstractNativeEventFilter *filterObject)
{
}

AbstractEventDispatcher *CoreApplication::eventDispatcher()
{
}

void CoreApplication::setEventDispatcher(AbstractEventDispatcher *eventDispatcher)
{
}

} // kernel
} // pdk
