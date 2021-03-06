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
// Created by softboy on 2017/01/12.

#include "gtest/gtest.h"
#include "pdk/stdext/optional/Optional.h"
#include <type_traits>

using pdk::stdext::optional::Optional;

struct X {};
struct Y {};
  
struct Resource
{
  explicit Resource(const X&) {}
};


TEST(OptionalSfinaeFriendluCtorTest, testSFINAE)
{
   PDK_STATIC_ASSERT((std::is_constructible<Resource, const X&>::value));
   PDK_STATIC_ASSERT((!std::is_constructible<Resource, const Y&>::value));
   
   PDK_STATIC_ASSERT((std::is_constructible<Optional<Resource>, const X&>::value));
   PDK_STATIC_ASSERT((!std::is_constructible<Optional<Resource>, const Y&>::value));
   
   PDK_STATIC_ASSERT((std::is_constructible<Optional<int>, Optional<int>>::value));
   PDK_STATIC_ASSERT((!std::is_constructible<Optional<int>, Optional<Optional<int>>>::value));
   
   PDK_STATIC_ASSERT((std::is_constructible<Optional<Optional<int>>, const Optional<int> &>::value));
   PDK_STATIC_ASSERT((!std::is_constructible<Optional<int>, const Optional<Optional<int>> &>::value));
   
   PDK_STATIC_ASSERT((std::is_constructible<Optional<Resource>, const Optional<X>&>::value));
   PDK_STATIC_ASSERT((!std::is_constructible<Optional<Resource>, const Optional<Y>&>::value));
}
