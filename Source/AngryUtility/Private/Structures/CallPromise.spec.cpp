// Maintained by AngryLizard, netliz.net

#include "Structures/CallPromise.h"

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

#include "Engine.h"

struct FTestError
{
};

template<typename PromiseType>
using TTestPromise = TCallPromise<FTestError, PromiseType>;

template<typename PromiseType>
using TTestPromisePtr = TSharedPtr<TTestPromise<PromiseType>>;


#pragma optimize("", off)
template<typename PromiseType>
TTestPromisePtr<PromiseType> AcceptLater(const PromiseType& Value)
{
    UE_LOG(LogTemp, Log, TEXT("AcceptLater called"));
    auto Promise = TTestPromise<PromiseType>::Create();
    AsyncTask(ENamedThreads::GameThread, [Promise, Value]()
    {
        UE_LOG(LogTemp, Log, TEXT("Accepting"));
        Promise->Accept(Value);
    });
    return Promise;
}

template<typename PromiseType>
TTestPromisePtr<PromiseType> RejectLater(const FTestError& Error)
{
    auto Promise = TTestPromise<PromiseType>::Create();
    AsyncTask(ENamedThreads::GameThread, [Promise, Error]()
    {
        Promise->Reject(Error);
    });
    return Promise;
}

DEFINE_SPEC(CallPromiseSpec, "Angry.CallPromiseSpec", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)
void CallPromiseSpec::Define()
{
    Describe("CallPromise::Accept", [this]()
    {
        LatentIt("should call 'Then' branch", EAsyncExecution::ThreadPool, [this](const FDoneDelegate& Done)
        {
            AcceptLater<int32>(5)
                ->Then([=](int32 Value)
                {
                    UE_LOG(LogTemp, Log, TEXT("Then"));
                    TestEqual("Then", Value, 5);
                })
                ->Then([=]()
                {
                    UE_LOG(LogTemp, Log, TEXT("ThenThen"));
                    TestTrue("ThenThen", true);
                })
                ->Catch([=](const FTestError& Error)
                {
                    UE_LOG(LogTemp, Log, TEXT("Catch"));
                    TestTrue("Catch", false);
                })
                ->Finally([=]()
                {
                    UE_LOG(LogTemp, Log, TEXT("Finally"));
                    TestTrue("Finally", true);
                    Done.Execute();
                });
        });

        LatentIt("should call immediate 'Then' branch", EAsyncExecution::ThreadPool, [this](const FDoneDelegate& Done)
        {
            TTestPromise<int32>::Accepted(5)
                ->Then([=](int32 Value)
                {
                    UE_LOG(LogTemp, Log, TEXT("Then"));
                    TestEqual("Then", Value, 5);
                })
                ->Catch([=](const FTestError& Error)
                {
                    UE_LOG(LogTemp, Log, TEXT("Catch"));
                    TestTrue("Catch", false);
                })
                ->Then([=]()
                {
                    UE_LOG(LogTemp, Log, TEXT("CatchThen"));
                    TestTrue("CatchThen", false);
                })
                ->Finally([=]()
                {
                    UE_LOG(LogTemp, Log, TEXT("Finally"));
                    TestTrue("Finally", true);
                    Done.Execute();
                });
        });

        LatentIt("should call chained 'Then' branches", EAsyncExecution::ThreadPool, [this](const FDoneDelegate& Done)
        {
            AcceptLater<int32>(5)
                ->Then([=](int32 Value)
                {
                    UE_LOG(LogTemp, Log, TEXT("Then"));
                    TestEqual("Then", Value, 5);
                    return AcceptLater<bool>(true);
                })
                ->Then([=](bool Value)
                {
                    UE_LOG(LogTemp, Log, TEXT("ThenThen"));
                    TestEqual("ThenThen", Value, true);
                    return AcceptLater<bool>(false);
                })
                ->Catch([=](const FTestError& Error)
                {
                    UE_LOG(LogTemp, Log, TEXT("Catch"));
                    TestTrue("Catch", false);
                    return AcceptLater<bool>(false); // Catch should not be called if not rejected
                })
                ->Then([=](bool Value)
                {
                    UE_LOG(LogTemp, Log, TEXT("CatchThen"));
                    TestTrue("CatchThen", false); // Then after catch should not be called if not rejected
                })
                ->Finally([=]()
                {
                    UE_LOG(LogTemp, Log, TEXT("Finally"));
                    TestTrue("Finally", true);
                    Done.Execute();
                });
        });
    });

    Describe("CallPromise::Reject", [this]()
    {
        LatentIt("should call 'Catch' branch", EAsyncExecution::ThreadPool, [this](const FDoneDelegate& Done)
        {
            RejectLater<int32>(FTestError())
                ->Then([=](int32 Value)
                {
                    UE_LOG(LogTemp, Log, TEXT("Then"));
                    TestTrue("Then", false);
                })
                ->Then([=]()
                {
                    UE_LOG(LogTemp, Log, TEXT("ThenThen"));
                    TestTrue("ThenThen", false);
                })
                ->Catch([=](const FTestError& Error)
                {
                    UE_LOG(LogTemp, Log, TEXT("Catch"));
                    TestTrue("Catch", true);
                })
                ->Finally([=]()
                {
                    UE_LOG(LogTemp, Log, TEXT("Finally"));
                    TestTrue("Finally", true);
                    Done.Execute();
                });
        });

        LatentIt("should call immediate 'Catch' branch", EAsyncExecution::ThreadPool, [this](const FDoneDelegate& Done)
        {
            TTestPromise<int32>::Rejected(FTestError())
                ->Then([=](int32 Value)
                {
                    UE_LOG(LogTemp, Log, TEXT("Then"));
                    TestTrue("Then", false);
                })
                ->Catch([=](const FTestError& Error)
                {
                    UE_LOG(LogTemp, Log, TEXT("Catch"));
                    TestTrue("Catch", true);
                })
                ->Then([=]()
                {
                    UE_LOG(LogTemp, Log, TEXT("CatchThen"));
                    TestTrue("CatchThen", true);
                })
                ->Finally([=]()
                {
                    UE_LOG(LogTemp, Log, TEXT("Finally"));
                    TestTrue("Finally", true);
                    Done.Execute();
                });
        });

        LatentIt("should call chained 'Catch' branches", EAsyncExecution::ThreadPool, [this](const FDoneDelegate& Done)
        {
            AcceptLater<int32>(5)
                ->Then([=](int32 Value)
                {
                    UE_LOG(LogTemp, Log, TEXT("Then"));
                    TestEqual("Then", Value, 5);
                    return RejectLater<bool>(FTestError());
                })
                ->Then([=](bool Value)
                {
                    UE_LOG(LogTemp, Log, TEXT("ThenThen"));
                    TestEqual("ThenThen", Value, true);
                    return AcceptLater<bool>(false); // Then should not be called if rejected
                })
                ->Catch([=](const FTestError& Error)
                {
                    UE_LOG(LogTemp, Log, TEXT("Catch"));
                    TestTrue("Catch", true);
                    return AcceptLater<FString>(TEXT("yes")); // Catch should be called if rejected
                })
                ->Then([=](const FString& Value)
                {
                    UE_LOG(LogTemp, Log, TEXT("CatchThen"));
                    TestEqual("CatchThen", Value, TEXT("yes")); // Then after catch should be called if rejected
                })
                ->Finally([=]()
                {
                    UE_LOG(LogTemp, Log, TEXT("Finally"));
                    TestTrue("Finally", true);
                    Done.Execute();
                });
        });
    });
}
#pragma optimize("", on)