// Maintained by AngryLizard, netliz.net

#pragma once

#include "CoreMinimal.h"
#include <type_traits>
#include <concepts>

template<typename ErrorType>
class TCallPromiseBase;

template<typename ErrorType, typename FuncType, typename ...Args>
concept IsPromisePtr = std::is_convertible_v<typename std::invoke_result<FuncType, Args...>::type, TSharedPtr<TCallPromiseBase<ErrorType>>>;

template<typename ErrorType, typename FuncType, typename ...Args>
concept IsVoidPtr = std::is_void_v<typename std::invoke_result<FuncType, Args...>::type>;

template<typename ErrorType, typename FuncType, typename ...Args>
concept IsValuePtr = not IsPromisePtr<ErrorType, FuncType, Args...> and not IsVoidPtr<ErrorType, FuncType, Args...>;

/**
* Conditions for SFINAE to differentiate between different lambda return types,
* because promise behaviour is different depending on whether the input lambda a) is another promise and b) has no return statement.
*/
template<typename ErrorType, typename ReturnType>
using EnableIfPromisePtr = std::enable_if_t<std::is_convertible_v<ReturnType, TSharedPtr<TCallPromiseBase<ErrorType>>>, void*>;

template<typename ErrorType, typename ReturnType>
using EnableIfValuePtr = std::enable_if_t<!std::is_void_v<ReturnType> && !std::is_convertible_v<ReturnType, TSharedPtr<TCallPromiseBase<ErrorType>>>, void*>;

template<typename ErrorType, typename ReturnType>
using EnableIfVoidPtr = std::enable_if_t<std::is_void_v<ReturnType> && !std::is_convertible_v<ReturnType, TSharedPtr<TCallPromiseBase<ErrorType>>>, void*>;

#define DeclReturnType_OneParam(F, T) decltype(Func(*(T*)nullptr))
#define DeclReturnType(F) decltype(Func())


template<typename ErrorType, typename PromiseType>
class TCallPromise;

template<typename ErrorType, typename PromiseType>
using TCallPromisePtr = TSharedPtr<TCallPromise<ErrorType, PromiseType>>;

/**
* Promise base includes the ability to reject a promise and catch rejected promises.
* Rejected promises will skip through all Then(...) calls until they reach a Catch(...).
*/
template<typename ErrorType>
class TCallPromiseBase : public TSharedFromThis<TCallPromiseBase<ErrorType>>
{
public:
	using FRejectDelegate = TDelegate<void(const ErrorType&)>;
	using FFinallyDelegate = TDelegate<void()>;

	void Reject(const ErrorType& Error);
	void Append(TSharedPtr<TCallPromiseBase> Promise, FRejectDelegate Reject);

	void Finalize();
	void Append(TSharedPtr<TCallPromiseBase> Promise, FFinallyDelegate Finally);

	template<typename FuncType>	requires IsPromisePtr<ErrorType, FuncType, ErrorType>	auto Catch(FuncType Func);
	template<typename FuncType>	requires IsValuePtr<ErrorType, FuncType, ErrorType>		auto Catch(FuncType Func);
	template<typename FuncType> requires IsVoidPtr<ErrorType, FuncType, ErrorType>		auto Catch(FuncType Func);
	template<typename UserClass, typename FuncType>	requires IsPromisePtr<ErrorType, FuncType, ErrorType>	auto WeakCatch(UserClass* Object, FuncType Func);
	template<typename UserClass, typename FuncType>	requires IsValuePtr<ErrorType, FuncType, ErrorType>		auto WeakCatch(UserClass* Object, FuncType Func);
	template<typename UserClass, typename FuncType> requires IsVoidPtr<ErrorType, FuncType, ErrorType>		auto WeakCatch(UserClass* Object, FuncType Func);
	
	template<typename FuncType>	requires IsPromisePtr<ErrorType, FuncType>	auto Finally(FuncType Func);
	template<typename FuncType>	requires IsValuePtr<ErrorType, FuncType>	auto Finally(FuncType Func);
	template<typename FuncType> requires IsVoidPtr<ErrorType, FuncType>		auto Finally(FuncType Func);
	template<typename UserClass, typename FuncType>	requires IsPromisePtr<ErrorType, FuncType, ErrorType>	auto WeakFinally(UserClass* Object, FuncType Func);
	template<typename UserClass, typename FuncType>	requires IsValuePtr<ErrorType, FuncType, ErrorType>		auto WeakFinally(UserClass* Object, FuncType Func);
	template<typename UserClass, typename FuncType> requires IsVoidPtr<ErrorType, FuncType, ErrorType>		auto WeakFinally(UserClass* Object, FuncType Func);

protected:
	bool bIsFinalized = false;
	TOptional<ErrorType> RejectError;

	FRejectDelegate RejectDelegate;
	FFinallyDelegate FinallyDelegate;
};

/**
* Generalized promise allows returning a value that will be provided in the future.
* Promises can be chained with Then(...) calls.
*/
template<typename ErrorType, typename PromiseType>
class TCallPromise : public TCallPromiseBase<ErrorType>
{
	template <typename, ESPMode>
	friend class SharedPointerInternals::TIntrusiveReferenceController; // Enable MakeShared

	template <typename, typename>
	friend class TCallPromise;

public:
	using FAcceptDelegate = TDelegate<void(const PromiseType&)>;

	static TCallPromisePtr<ErrorType, PromiseType> Create();
	static TCallPromisePtr<ErrorType, PromiseType> Accepted(const PromiseType& Value);
	static TCallPromisePtr<ErrorType, PromiseType> Rejected(const ErrorType& Error);

	void Accept(const PromiseType& Value);
	void Merge(TCallPromisePtr<ErrorType, PromiseType> Promise);
	void Append(TSharedPtr<TCallPromiseBase<ErrorType>> Promise, FAcceptDelegate Accept);

	template<typename FuncType>	requires IsPromisePtr<ErrorType, FuncType, PromiseType>		auto Then(FuncType Func);
	template<typename FuncType>	requires IsValuePtr<ErrorType, FuncType, PromiseType>		auto Then(FuncType Func);
	template<typename FuncType> requires IsVoidPtr<ErrorType, FuncType, PromiseType>		auto Then(FuncType Func);
	template<typename UserClass, typename FuncType>	requires IsPromisePtr<ErrorType, FuncType, PromiseType>		auto WeakThen(UserClass* Object, FuncType Func);
	template<typename UserClass, typename FuncType>	requires IsValuePtr<ErrorType, FuncType, PromiseType>		auto WeakThen(UserClass* Object, FuncType Func);
	template<typename UserClass, typename FuncType> requires IsVoidPtr<ErrorType, FuncType, PromiseType>		auto WeakThen(UserClass* Object, FuncType Func);

private:
	TCallPromise() {}
	TOptional<PromiseType> AcceptValue;

	FAcceptDelegate AcceptDelegate;
};

/**
* Empty promise spezialization for functions that return nothing.
*/
template<typename ErrorType>
class TCallPromise<ErrorType, void> : public TCallPromiseBase<ErrorType>
{
	template <typename, ESPMode>
	friend class SharedPointerInternals::TIntrusiveReferenceController; // Enable MakeShared

	template <typename, typename>
	friend class TCallPromise;

public:
	using FAcceptDelegate = TDelegate<void()>;

	static TCallPromisePtr<ErrorType, void> Create();
	static TCallPromisePtr<ErrorType, void> Accepted();
	static TCallPromisePtr<ErrorType, void> Rejected(const ErrorType& Error);

	void Accept();
	void Merge(TCallPromisePtr<ErrorType, void> Promise);
	void Append(TSharedPtr<TCallPromiseBase<ErrorType>> Promise, FAcceptDelegate Accept);

	template<typename FuncType>	requires IsPromisePtr<ErrorType, FuncType>	auto Then(FuncType Func);
	template<typename FuncType>	requires IsValuePtr<ErrorType, FuncType>	auto Then(FuncType Func);
	template<typename FuncType> requires IsVoidPtr<ErrorType, FuncType>		auto Then(FuncType Func);
	template<typename UserClass, typename FuncType>	requires IsPromisePtr<ErrorType, FuncType>	auto WeakThen(UserClass* Object, FuncType Func);
	template<typename UserClass, typename FuncType>	requires IsValuePtr<ErrorType, FuncType>	auto WeakThen(UserClass* Object, FuncType Func);
	template<typename UserClass, typename FuncType> requires IsVoidPtr<ErrorType, FuncType>		auto WeakThen(UserClass* Object, FuncType Func);

private:
	TCallPromise() {}
	bool bIsAccepted = false;

	FAcceptDelegate AcceptDelegate;
};

// TCallPromiseBase Implementation

template<typename ErrorType>
void TCallPromiseBase<ErrorType>::Reject(const ErrorType& Error)
{
	this->RejectError = Error;
	if (this->RejectDelegate.IsBound())
	{
		this->RejectDelegate.ExecuteIfBound(Error);
		this->RejectDelegate.Unbind();
	}
	else
	{
		this->Finalize();
	}
}

template<typename ErrorType>
void TCallPromiseBase<ErrorType>::Finalize()
{
	this->bIsFinalized = true;
	this->FinallyDelegate.ExecuteIfBound();
	this->FinallyDelegate.Unbind();
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

template<typename ErrorType>
template<typename FuncType>
requires IsPromisePtr<ErrorType, FuncType, ErrorType>
auto TCallPromiseBase<ErrorType>::Catch(FuncType Func)
{
	using ReturnPromiseType = std::invoke_result<FuncType, ErrorType>::type::ElementType;
	TSharedPtr<ReturnPromiseType> Promise = ReturnPromiseType::Create();

	this->Append(Promise, FRejectDelegate::CreateLambda(
		[Func = MoveTemp(Func), Promise](const ErrorType& Error)
	{
		Promise->Merge(Func(Error));
	}));

	return Promise;
}

template<typename ErrorType>
template<typename FuncType>
requires IsValuePtr<ErrorType, FuncType, ErrorType>
auto TCallPromiseBase<ErrorType>::Catch(FuncType Func)
{
	using ReturnType = std::invoke_result<FuncType, ErrorType>::type;
	TCallPromisePtr<ErrorType, ReturnType> Promise = TCallPromise<ErrorType, ReturnType>::Create();

	this->Append(Promise, FRejectDelegate::CreateLambda(
		[Func = MoveTemp(Func), Promise](const ErrorType& Error)
	{
		Promise->Accept(Func(Error));
	}));

	return Promise;
}

template<typename ErrorType>
template<typename FuncType>
requires IsVoidPtr<ErrorType, FuncType, ErrorType>
auto TCallPromiseBase<ErrorType>::Catch(FuncType Func)
{
	TCallPromisePtr<ErrorType, void> Promise = TCallPromise<ErrorType, void>::Create();

	this->Append(Promise, FRejectDelegate::CreateLambda(
		[Func = MoveTemp(Func), Promise](const ErrorType& Error)
	{
		Func(Error);
		Promise->Accept();
	}));

	return Promise;
}

template<typename ErrorType>
template<typename UserClass, typename FuncType>
requires IsPromisePtr<ErrorType, FuncType, ErrorType>
auto TCallPromiseBase<ErrorType>::WeakCatch(UserClass* Object, FuncType Func)
{
	using ReturnPromiseType = std::invoke_result<FuncType, ErrorType>::type::ElementType;
	TSharedPtr<ReturnPromiseType> Promise = ReturnPromiseType::Create();

	this->Append(Promise, FRejectDelegate::CreateWeakLambda(Object,
		[Func = MoveTemp(Func), Promise](const ErrorType& Error)
	{
		Promise->Merge(Func(Error));
	}));

	return Promise;
}

template<typename ErrorType>
template<typename UserClass, typename FuncType>
requires IsValuePtr<ErrorType, FuncType, ErrorType>
auto TCallPromiseBase<ErrorType>::WeakCatch(UserClass* Object, FuncType Func)
{
	using ReturnType = std::invoke_result<FuncType, ErrorType>::type;
	TCallPromisePtr<ErrorType, ReturnType> Promise = TCallPromise<ErrorType, ReturnType>::Create();

	this->Append(Promise, FRejectDelegate::CreateWeakLambda(Object,
		[Func = MoveTemp(Func), Promise](const ErrorType& Error)
	{
		Promise->Accept(Func(Error));
	}));

	return Promise;
}

template<typename ErrorType>
template<typename UserClass, typename FuncType>
requires IsVoidPtr<ErrorType, FuncType, ErrorType>
auto TCallPromiseBase<ErrorType>::WeakCatch(UserClass* Object, FuncType Func)
{
	TCallPromisePtr<ErrorType, void> Promise = TCallPromise<ErrorType, void>::Create();

	this->Append(Promise, FRejectDelegate::CreateWeakLambda(Object,
		[Func = MoveTemp(Func), Promise](const ErrorType& Error)
	{
		Func(Error);
		Promise->Accept();
	}));

	return Promise;
}

template<typename ErrorType>
void TCallPromiseBase<ErrorType>::Append(TSharedPtr<TCallPromiseBase> Promise, FRejectDelegate Reject)
{
	if (this->RejectError.IsSet())
	{
		Reject.ExecuteIfBound(this->RejectError.GetValue());
		return;
	}

	if (this->bIsFinalized)
	{
		Promise->Finalize();
		return;
	}

	check(!this->RejectDelegate.IsBound());
	this->RejectDelegate = MoveTemp(Reject);

	check(!this->FinallyDelegate.IsBound());
	this->FinallyDelegate = FFinallyDelegate::CreateLambda([Promise]()
	{
		Promise->Finalize();
	});
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

template<typename ErrorType>
template<typename FuncType>
requires IsPromisePtr<ErrorType, FuncType>
auto TCallPromiseBase<ErrorType>::Finally(FuncType Func)
{
	using ReturnPromiseType = std::invoke_result<FuncType>::type::ElementType;
	TSharedPtr<ReturnPromiseType> Promise = ReturnPromiseType::Create();

	this->Append(Promise, FFinallyDelegate::CreateLambda(
		[Func = MoveTemp(Func), Promise]()
	{
		Promise->Merge(Func());
	}));

	return Promise;
}

template<typename ErrorType>
template<typename FuncType>
requires IsValuePtr<ErrorType, FuncType>
auto TCallPromiseBase<ErrorType>::Finally(FuncType Func)
{
	using ReturnType = std::invoke_result<FuncType>::type;
	TCallPromisePtr<ErrorType, ReturnType> Promise = TCallPromise<ErrorType, ReturnType>::Create();

	this->Append(Promise, FFinallyDelegate::CreateLambda(
		[Func = MoveTemp(Func), Promise]()
	{
		Promise->Accept(Func());
	}));

	return Promise;
}

template<typename ErrorType>
template<typename FuncType>
requires IsVoidPtr<ErrorType, FuncType>
auto TCallPromiseBase<ErrorType>::Finally(FuncType Func)
{
	TCallPromisePtr<ErrorType, void> Promise = TCallPromise<ErrorType, void>::Create();

	this->Append(Promise, FFinallyDelegate::CreateLambda(
		[Func = MoveTemp(Func), Promise]()
	{
		Func();
		Promise->Accept();
	}));

	return Promise;
}

template<typename ErrorType>
template<typename UserClass, typename FuncType>
requires IsPromisePtr<ErrorType, FuncType, ErrorType>
auto TCallPromiseBase<ErrorType>::WeakFinally(UserClass* Object, FuncType Func)
{
	using ReturnPromiseType = std::invoke_result<FuncType>::type::ElementType;
	TSharedPtr<ReturnPromiseType> Promise = ReturnPromiseType::Create();

	this->Append(Promise, FFinallyDelegate::CreateWeakLambda(Object,
		[Func = MoveTemp(Func), Promise]()
	{
		Promise->Merge(Func());
	}));

	return Promise;
}

template<typename ErrorType>
template<typename UserClass, typename FuncType>
requires IsValuePtr<ErrorType, FuncType, ErrorType>
auto TCallPromiseBase<ErrorType>::WeakFinally(UserClass* Object, FuncType Func)
{
	using ReturnType = std::invoke_result<FuncType>::type;
	TCallPromisePtr<ErrorType, ReturnType> Promise = TCallPromise<ErrorType, ReturnType>::Create();

	this->Append(Promise, FFinallyDelegate::CreateWeakLambda(Object,
		[Func = MoveTemp(Func), Promise]()
	{
		Promise->Accept(Func());
	}));

	return Promise;
}

template<typename ErrorType>
template<typename UserClass, typename FuncType>
requires IsVoidPtr<ErrorType, FuncType, ErrorType>
auto TCallPromiseBase<ErrorType>::WeakFinally(UserClass* Object, FuncType Func)
{
	TCallPromisePtr<ErrorType, void> Promise = TCallPromise<ErrorType, void>::Create();

	this->Append(Promise, FFinallyDelegate::CreateWeakLambda(Object,
		[Func = MoveTemp(Func), Promise]()
	{
		Func();
		Promise->Accept();
	}));

	return Promise;
}

template<typename ErrorType>
void TCallPromiseBase<ErrorType>::Append(TSharedPtr<TCallPromiseBase> Promise, FFinallyDelegate Finally)
{
	if (this->bIsFinalized)
	{
		Finally.ExecuteIfBound();
		return;
	}

	check(!this->FinallyDelegate.IsBound());
	this->FinallyDelegate = MoveTemp(Finally);
}

// TCallPromise<ErrorType, PromiseType> Implementation

template<typename ErrorType, typename PromiseType>
TCallPromisePtr<ErrorType, PromiseType> TCallPromise<ErrorType, PromiseType>::Create()
{
	return MakeShared<TCallPromise<ErrorType, PromiseType>>();
}

template<typename ErrorType, typename PromiseType>
TCallPromisePtr<ErrorType, PromiseType> TCallPromise<ErrorType, PromiseType>::Accepted(const PromiseType& Value)
{
	TCallPromisePtr<ErrorType, PromiseType> Promise = Create();
	Promise->Accept(Value);
	return Promise;
}

template<typename ErrorType, typename PromiseType>
TCallPromisePtr<ErrorType, PromiseType> TCallPromise<ErrorType, PromiseType>::Rejected(const ErrorType& Error)
{
	TCallPromisePtr<ErrorType, PromiseType> Promise = Create();
	Promise->Reject(Error);
	return Promise;
}

template<typename ErrorType, typename PromiseType>
void TCallPromise<ErrorType, PromiseType>::Accept(const PromiseType& Value)
{
	this->AcceptValue = Value;
	if (this->AcceptDelegate.IsBound())
	{
		this->AcceptDelegate.ExecuteIfBound(Value);
		this->AcceptDelegate.Unbind();
	}
	else
	{
		this->Finalize();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

template<typename ErrorType, typename PromiseType>
template<typename FuncType>
requires IsPromisePtr<ErrorType, FuncType, PromiseType>
auto TCallPromise<ErrorType, PromiseType>::Then(FuncType Func)
{
	using ReturnPromiseType = std::invoke_result<FuncType, PromiseType>::type::ElementType;
	TSharedPtr<ReturnPromiseType> Promise = ReturnPromiseType::Create();

	this->Append(Promise, FAcceptDelegate::CreateLambda(
		[Func = MoveTemp(Func), Promise](const PromiseType& Result)
	{
		Promise->Merge(Func(Result));
	}));

	return Promise;
}

template<typename ErrorType, typename PromiseType>
template<typename FuncType>
requires IsValuePtr<ErrorType, FuncType, PromiseType>
auto TCallPromise<ErrorType, PromiseType>::Then(FuncType Func)
{
	using ReturnType = std::invoke_result<FuncType, PromiseType>::type;
	TCallPromisePtr<ErrorType, ReturnType> Promise = TCallPromise<ErrorType, ReturnType>::Create();

	this->Append(Promise, FAcceptDelegate::CreateLambda(
		[Func = MoveTemp(Func), Promise](const PromiseType& Result)
	{
		Promise->Accept(Func(Result));
	}));

	return Promise;
}

template<typename ErrorType, typename PromiseType>
template<typename FuncType>
requires IsVoidPtr<ErrorType, FuncType, PromiseType>
auto TCallPromise<ErrorType, PromiseType>::Then(FuncType Func)
{
	TCallPromisePtr<ErrorType, void> Promise = TCallPromise<ErrorType, void>::Create();

	this->Append(Promise, FAcceptDelegate::CreateLambda(
		[Func = MoveTemp(Func), Promise](const PromiseType& Result)
	{
		Func(Result);
		Promise->Accept();
	}));

	return Promise;
}

template<typename ErrorType, typename PromiseType>
template<typename UserClass, typename FuncType>
requires IsPromisePtr<ErrorType, FuncType, PromiseType>
auto TCallPromise<ErrorType, PromiseType>::WeakThen(UserClass* Object, FuncType Func)
{
	using ReturnPromiseType = std::invoke_result<FuncType, PromiseType>::type::ElementType;
	TSharedPtr<ReturnPromiseType> Promise = ReturnPromiseType::Create();

	this->Append(Promise, FAcceptDelegate::CreateWeakLambda(Object,
		[Func = MoveTemp(Func), Promise](const PromiseType& Result)
	{
		Promise->Merge(Func(Result));
	}));

	return Promise;
}

template<typename ErrorType, typename PromiseType>
template<typename UserClass, typename FuncType>
requires IsValuePtr<ErrorType, FuncType, PromiseType>
auto TCallPromise<ErrorType, PromiseType>::WeakThen(UserClass* Object, FuncType Func)
{
	using ReturnType = std::invoke_result<FuncType, PromiseType>::type;
	TCallPromisePtr<ErrorType, ReturnType> Promise = TCallPromise<ErrorType, ReturnType>::Create();

	this->Append(Promise, FAcceptDelegate::CreateWeakLambda(Object,
		[Func = MoveTemp(Func), Promise](const PromiseType& Result)
	{
		Promise->Accept(Func(Result));
	}));

	return Promise;
}

template<typename ErrorType, typename PromiseType>
template<typename UserClass, typename FuncType>
requires IsVoidPtr<ErrorType, FuncType, PromiseType>
auto TCallPromise<ErrorType, PromiseType>::WeakThen(UserClass* Object, FuncType Func)
{
	TCallPromisePtr<ErrorType, void> Promise = TCallPromise<ErrorType, void>::Create();

	this->Append(Promise, FAcceptDelegate::CreateWeakLambda(Object,
		[Func = MoveTemp(Func), Promise](const PromiseType& Result)
	{
		Func(Result);
		Promise->Accept();
	}));

	return Promise;
}

template<typename ErrorType, typename PromiseType>
void TCallPromise<ErrorType, PromiseType>::Append(TSharedPtr<TCallPromiseBase<ErrorType>> Promise, FAcceptDelegate Accept)
{
	if (this->AcceptValue.IsSet())
	{
		Accept.ExecuteIfBound(this->AcceptValue.GetValue());
		return;
	}

	if (this->RejectError.IsSet())
	{
		Promise->Reject(this->RejectError.GetValue());
		return;
	}

	if (this->bIsFinalized)
	{
		Promise->Finalize();
		return;
	}

	check(!this->AcceptDelegate.IsBound());
	this->AcceptDelegate = MoveTemp(Accept);

	check(!this->RejectDelegate.IsBound());
	this->RejectDelegate = TCallPromiseBase<ErrorType>::FRejectDelegate::CreateLambda([Promise](const ErrorType& Error)
	{
		Promise->Reject(Error);
	});

	check(!this->FinallyDelegate.IsBound());
	this->FinallyDelegate = TCallPromiseBase<ErrorType>::FFinallyDelegate::CreateLambda([Promise]()
	{
		Promise->Finalize();
	});
}

template<typename ErrorType, typename PromiseType>
void TCallPromise<ErrorType, PromiseType>::Merge(TCallPromisePtr<ErrorType, PromiseType> Promise)
{
	if (Promise->RejectError.IsSet())
	{
		this->Reject(Promise->RejectError.GetValue());
		return;
	}

	if (Promise->AcceptValue.IsSet())
	{
		Accept(Promise->AcceptValue.GetValue());
		return;
	}

	if (Promise->bIsFinalized)
	{
		this->Finalize();
		return;
	}

	check(!Promise->AcceptDelegate.IsBound());
	Promise->AcceptDelegate = FAcceptDelegate::CreateLambda([this, SelfPromise = this->AsShared()](const PromiseType& Result)
	{
		// SelfPromise prevents deletion
		this->Accept(Result);
	});

	check(!Promise->RejectDelegate.IsBound());
	Promise->RejectDelegate = TCallPromiseBase<ErrorType>::FRejectDelegate::CreateLambda([SelfPromise = this->AsShared()](const ErrorType& Error)
	{
		SelfPromise->Reject(Error);
	});

	check(!Promise->FinallyDelegate.IsBound());
	Promise->FinallyDelegate = TCallPromiseBase<ErrorType>::FFinallyDelegate::CreateLambda([SelfPromise = this->AsShared()]()
	{
		SelfPromise->Finalize();
	});
}

// TCallPromise<ErrorType, void> Implementation

template<typename ErrorType>
TCallPromisePtr<ErrorType, void> TCallPromise<ErrorType, void>::Create()
{
	return MakeShared<TCallPromise<ErrorType, void>>();
}

template<typename ErrorType>
TCallPromisePtr<ErrorType, void> TCallPromise<ErrorType, void>::Accepted()
{
	TCallPromisePtr<ErrorType, void> Promise = this->Create();
	Promise->Accept();
	return Promise;
}

template<typename ErrorType>
TCallPromisePtr<ErrorType, void> TCallPromise<ErrorType, void>::Rejected(const ErrorType& Error)
{
	TCallPromisePtr<ErrorType, void> Promise = this->Create();
	Promise->Reject(Error);
	return Promise;
}

template<typename ErrorType>
void TCallPromise<ErrorType, void>::Accept()
{
	this->bIsAccepted = true;
	if (this->AcceptDelegate.IsBound())
	{
		this->AcceptDelegate.ExecuteIfBound();
		this->AcceptDelegate.Unbind();
	}
	else
	{
		this->Finalize();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

template<typename ErrorType>
template<typename FuncType>
requires IsPromisePtr<ErrorType, FuncType>
auto TCallPromise<ErrorType, void>::Then(FuncType Func)
{
	using ReturnPromiseType = std::invoke_result<FuncType>::type::ElementType;
	TSharedPtr<ReturnPromiseType> Promise = ReturnPromiseType::Create();

	this->Append(Promise, FAcceptDelegate::CreateLambda(
		[Func = MoveTemp(Func), Promise]()
	{
		Promise->Merge(Func());
	}));

	return Promise;
}

template<typename ErrorType>
template<typename FuncType>
requires IsValuePtr<ErrorType, FuncType>
auto TCallPromise<ErrorType, void>::Then(FuncType Func)
{
	using ReturnType = std::invoke_result<FuncType>::type;
	TCallPromisePtr<ErrorType, ReturnType> Promise = TCallPromise<ErrorType, ReturnType>::Create();

	this->Append(Promise, FAcceptDelegate::CreateLambda(
		[Func = MoveTemp(Func), Promise]()
	{
		Promise->Accept(Func());
	}));

	return Promise;
}

template<typename ErrorType>
template<typename FuncType>
requires IsVoidPtr<ErrorType, FuncType>
auto TCallPromise<ErrorType, void>::Then(FuncType Func)
{
	TCallPromisePtr<ErrorType, void> Promise = TCallPromise<ErrorType, void>::Create();

	this->Append(Promise, FAcceptDelegate::CreateLambda(
		[Func = MoveTemp(Func), Promise]()
	{
		Func();
		Promise->Accept();
	}));

	return Promise;
}

template<typename ErrorType>
template<typename UserClass, typename FuncType>
requires IsPromisePtr<ErrorType, FuncType>
auto TCallPromise<ErrorType, void>::WeakThen(UserClass* Object, FuncType Func)
{
	using ReturnPromiseType = std::invoke_result<FuncType>::type::ElementType;
	TSharedPtr<ReturnPromiseType> Promise = ReturnPromiseType::Create();

	this->Append(Promise, FAcceptDelegate::CreateWeakLambda(Object,
		[Func = MoveTemp(Func), Promise]()
	{
		Promise->Merge(Func());
	}));

	return Promise;
}

template<typename ErrorType>
template<typename UserClass, typename FuncType>
requires IsValuePtr<ErrorType, FuncType>
auto TCallPromise<ErrorType, void>::WeakThen(UserClass* Object, FuncType Func)
{
	using ReturnType = std::invoke_result<FuncType>::type;
	TCallPromisePtr<ErrorType, ReturnType> Promise = TCallPromise<ErrorType, ReturnType>::Create();

	this->Append(Promise, FAcceptDelegate::CreateWeakLambda(Object,
		[Func = MoveTemp(Func), Promise]()
	{
		Promise->Accept(Func());
	}));

	return Promise;
}

template<typename ErrorType>
template<typename UserClass, typename FuncType>
requires IsVoidPtr<ErrorType, FuncType>
auto TCallPromise<ErrorType, void>::WeakThen(UserClass* Object, FuncType Func)
{
	TCallPromisePtr<ErrorType, void> Promise = TCallPromise<ErrorType, void>::Create();

	this->Append(Promise, FAcceptDelegate::CreateWeakLambda(Object,
		[Func = MoveTemp(Func), Promise]()
	{
		Func();
		Promise->Accept();
	}));

	return Promise;
}

template<typename ErrorType>
void TCallPromise<ErrorType, void>::Append(TSharedPtr<TCallPromiseBase<ErrorType>> Promise, FAcceptDelegate Accept)
{
	if (this->bIsAccepted)
	{
		Accept.ExecuteIfBound();
		return;
	}

	if (this->RejectError.IsSet())
	{
		Promise->Reject(this->RejectError.GetValue());
		return;
	}

	if (this->bIsFinalized)
	{
		Promise->Finalize();
		return;
	}

	check(!this->AcceptDelegate.IsBound());
	this->AcceptDelegate = MoveTemp(Accept);

	check(!this->RejectDelegate.IsBound());
	this->RejectDelegate = TCallPromiseBase<ErrorType>::FRejectDelegate::CreateLambda([Promise](const ErrorType& Error)
	{
		Promise->Reject(Error);
	});

	check(!this->FinallyDelegate.IsBound());
	this->FinallyDelegate = TCallPromiseBase<ErrorType>::FFinallyDelegate::CreateLambda([Promise]()
	{
		Promise->Finalize();
	});
}

template<typename ErrorType>
void TCallPromise<ErrorType, void>::Merge(TCallPromisePtr<ErrorType, void> Promise)
{
	if (Promise->bIsAccepted)
	{
		Accept();
		return;
	}

	if (Promise->RejectError.IsSet())
	{
		this->Reject(Promise->RejectError.GetValue());
		return;
	}

	if (Promise->bIsFinalized)
	{
		this->Finalize();
		return;
	}

	check(!Promise->AcceptDelegate.IsBound());
	Promise->AcceptDelegate = FAcceptDelegate::CreateLambda([this, SelfPromise = this->AsShared()]()
	{
		// SelfPromise prevents deletion
		Accept();
	});

	check(!Promise->RejectDelegate.IsBound());
	Promise->RejectDelegate = TCallPromiseBase<ErrorType>::FRejectDelegate::CreateLambda([SelfPromise = this->AsShared()](const ErrorType& Error)
	{
		SelfPromise->Reject(Error);
	});

	check(!Promise->FinallyDelegate.IsBound());
	Promise->FinallyDelegate = TCallPromiseBase<ErrorType>::FFinallyDelegate::CreateLambda([SelfPromise = this->AsShared()]()
	{
		SelfPromise->Finalize();
	});
}
