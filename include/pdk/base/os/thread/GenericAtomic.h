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
// Created by softboy on 2017/11/23.

#ifndef PDK_M_BASE_OS_THREAD_GENERIC_ATOMIC_H
#define PDK_M_BASE_OS_THREAD_GENERIC_ATOMIC_H

namespace pdk {
namespace os {
namespace thread {

template <int>
struct AtomicOpsSupport
{
   enum {
      IsSupported = 0
   };
};

template <>
struct AtomicOpsSupport<4>
{
   enum {
      IsSupported = 1
   };
};

template <typename T>
struct AtomicAdditiveType
{
   using AdditiveType = T;
   static const int AddScale = 1;
};

template <typename T>
struct AtomicAdditiveType<T *>
{
   using AdditiveType = T;
   static const int AddScale = sizeof(T);
};

} // thread
} // os
} // pdk

#endif // PDK_M_BASE_OS_THREAD_GENERIC_ATOMIC_H
