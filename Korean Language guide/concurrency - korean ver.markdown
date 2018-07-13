^title 동시성(Concurrency)

경량 동시성은 Wren의 주요 특징이며 *fibers*로 표현됩니다. 그들은 모든 코드가 어떻게 실행되는지를 제어하고, [오류 처리](error-handling.html)에서의 예외 상황을 대처합니다.

Fibers는 *상호협력적*으로 짜인다는 것을 제외하면 스레드와 비슷합니다. 이는
당신이 명령하기 전까지 Wren이 한 fiber를 멈추고 다른 fiber로 전환하지 않는다는 의미입니다. 
당신은 예측하지 못할 때에 일어나는 문맥 교환과 그것으로 인한 모든 골칫거리들을 걱정할 필요가 없습니다.

Wren은 VM의 모든 fiber를 관리하므로 OS스레드 리소스를 사용하거나 무거운 문맥 교환을 필요로 하지 않습니다.
각각은 단지 그것의 스택을 위한 조금의 메모리가 필요할 뿐입니다.
Fiber는 더이상 참조되지 않을 때, 다른 객체처럼 가비지(garbage)를 수거할 것입니다.
그래서 당신은 그것들을 자유롭게 만들어 낼 수 있습니다.

Fiber는 당신이 예를 들어 별도의 fiber를 게임의 각 개체에 이용할 수 있을 만큼 경량입니다.
Wren은 수천 개의 fibers를 큰 어려움 없이 다룰 수 있습니다.
예를 들어, 당신이 Wren을 인터랙티브 모드로 이용할 때, 당신이 입력하는 모든 줄의 코드를 위해 새로운 fiber를 만들어냅니다.

## Fibers 만들어내기

모든 Wren 코드는 fiber의 문맥 안에서 실행됩니다. 당신이 처음 Wren을 시작할 때, 메인 fiber가 자동으로 생성됩니다. 당신은 fiber 클래스의 생성자 사용으로 새로운 fiber를 생성할 수 있습니다:

    :::wren
    var fiber = Fiber.new {
      System.print("This runs in a separate fiber.")
    }
	
fiber가 실행해야 하는 코드를 포함하는[함수][]가 필요합니다. 그
함수는 0 또는 1개의 매개 변수를 사용할 수 있지만, 그 이상은 사용할 수 없습니다.
fiber를 만든다고 즉시 실행되지는 않습니다. fiber는 그저 함수를 감싸고 활성화되기를 기다리고 있습니다. 

[함수]:functions.html

## Fibers 호출하기

Fiber를 생성한 후 당신은 `call()` 메소드를 이용하여 호출합니다.:

    :::wren
    fiber.call()

이것은 현재 fiber를 중단하고 호출된 fiber가 그것의 바디의 끝에 도달할 때까지 또는 그것이 다른 fiber에게 통제를 넘기기 전까지 실행합니다.
만약 그것이 바디의 끝에 도달하면 이는 *완료*로 여겨집니다.:

    :::wren
    var fiber = Fiber.new {
      System.print("It's alive!")
    }

    System.print(fiber.isDone) //> false
    fiber.call() //> It's alive!
    System.print(fiber.isDone) //> true

호출된 fiber가 종료되면, 그것은 자동으로 통제를 그것을 부른 *이전* fiber로 넘깁니다.
이미 완료된 fiber를 호출하려 하는 것은 런타임 오류입니다.

## yielding

Fiber와 함수의 중요한 차이점은 fiber는 이것의 작동 중에 중단될 수 있으며 나중에 해당 지점에서 재시작할 수 있다는 것입니다. 
다른 fiber를 호출하는 것은 fiber를 중단하는 방법의 하나지만 그것은 한 함수가 다른 함수를 호출하는 것과 거의 같습니다.

fiber가 *yields*되었을 때가 더욱 특수합니다. yield된 fiber는 통제를 그것을 실행한 *이전의* fiber에게 넘기지만, *자신이 어디에 있는지 기억합니다*.
다음번에 그 fiber가 호출되면, 그것은 중단된 시점을 찾아 해당 시점부터 진행합니다.

당신은 fiber yield를 `yield()` 정적 메소드를 fiber에 대해 실행하여 만듭니다.:

    :::wren
    var fiber = Fiber.new {
      System.print("Before yield")
      Fiber.yield()
      System.print("Resumed")
    }

    System.print("Before call") //> Before call
    fiber.call() //> Before yield
    System.print("Calling again") //> Calling again
    fiber.call() //> Resumed
    System.print("All done") //> All done

이 프로그램이 *동시성*을 사용함에도 불구하고, 여전히
*결정론적*임을 참고하세요. 당신은 정확히 무엇을 하고 있는지 추론할 수 있으며 스레드 스케줄러가 당신의 코드를 가지고 러시안 룰렛을 하도록 두지 않습니다.

## 값 전달하기

호출하기와 fiber를 yielding하는 것은 통제를 넘기는 데 사용됩니다. 그러나 이는 또한 
*데이터*를 넘기는데에도 사용됩니다. 당신이 fiber를 호출할 때, 당신은 선택적으로 값을 그것에 전달할 수 있습니다.

만약 당신이 fiber를 매개변수를 가지는 함수를 이용하여 생성하였다면, 당신은 값을
`call()`을 통해 전달할 수 있습니다.:

    :::wren
    var fiber = Fiber.new {|param|
      System.print(param)
    }

    fiber.call("Here you go") //> Here you go

만약 fiber yield 되었고 재시작을 기다리고 있다면, 당신이 호출하기 위해 전달한 값은
그것이 재시작될 때 `yield()` 호출의 리턴값이 될 것입니다.:

    :::wren
    var fiber = Fiber.new {|param|
      System.print(param)
      var result = Fiber.yield()
      System.print(result)
    }

    fiber.call("First") //> First
    fiber.call("Second") //> Second

Fibers는 또한 그들이 yield 되었을 때 값을 *뒤로* 전달할 수 있습니다. 만약 당신이
`yield()`에 인수를 전달하였다면, 그것은 `call()`의 fiber를 invoke하는데 사용된 리턴값이 될 것이다.

    :::wren
    var fiber = Fiber.new {
      Fiber.yield("Reply")
    }

    System.print(fiber.call()) //> Reply

이것은 함수 호출이 값을 반환하는 것과 비슷합니다. 단, fiber는 하나의 값이 yield될 때마다 값의 전체 시퀀스를 반환합니다.

## 전체 제어

우리가 지금까지 본 것은 당신이 *생성자*를 가지는 파이썬이나 C#같은 언어로 할 수 있는 것과 매우 유사합니다.
그것들은 당신이 중단하고 재시작이 가능한 함수 호출을 정의할 수 있게 해줍니다.
함수를 사용할 때, 그것은 반복 가능한 시퀀스처럼 보입니다.

Wren의 fibers는 훨씬 더 많은 것들을 할 수 있습니다. Lua와 비슷하게,
*코루틴(coroutines)*으로 가득합니다.&mdash; 그들은 호출 스택 어디서든 중단 가능합니다. 
당신이 fiber를 생성하기 위해 사용한 함수는 최종 yield를 호출하는 세 번째 메소드를 호출하는 다른 메소드를 호출하는 메소드를 
호출할 수 있습니다. 그때, *모든* 메소드 호출은
&mdash; 전체 호출 스택 &mdash;중단됩니다. 예를 들어:

    :::wren
    var fiber = Fiber.new {
      (1..10).each {|i|
        Fiber.yield(i)
      }
    }

여기서 우리는 `each()` 메소드로 전달된 [function] (functions.html) 내에서 yield()를 호출하고 있습니다.
이 작업은 Wren에서는 작동합니다. 왜냐하면 안쪽
`yield()` 호출은 `each()`에 대한 호출과 콜백으로 그것에게 전달된 함수를 일시 중단하기 때문입니다.

## 전송 제어

Fibers는 비장의 카드를 하나 더 가집니다. 
`call()`을 사용하여 당신이 fiber를 실행시키면, 그 fiber는 어떤 fiber를 그것이 yeild될 때 리턴할 것인지를 추적합니다.
그것은 모든 호출된 fiber가 yeild되거나 완료되면 결국 주 fiber로 돌아가는 fiber 호출의 체인을 구축하도록 해줍니다.

이것은 보통 당신이 원하는 것입니다. 하지만 fiber들을 관리하기 위한 당신의 스케줄러를 만드는 것과 같이 당신이 뭔가를 낮은 수준으로 하고 있다면,
당신은 그것들을 스택처럼 명시적으로 다루지 않기를 원할 수 있습니다.

그런 드문 경우를 위해, fibers는 또한 `transfer()` 메소드를 가집니다. 이것은 실행을
전달된 fiber로 전환하고 *어딘가에서* 전달된 fiber를 "잊습니다".
이전 상태는 어떤 단계에 있든 간에 일시 중지됩니다.
당신은 다시 명시적으로 fiber로 전송하거나 심지어 그것을 호출함으로써 이전 fiber를 재시작할 수 있습니다. 만약 당신이 그렇게 하지 않으면 마지막으로 전송된 fiber가 반환될 때 실행은 몀춥니다.

`call()`과 `yield()`는 함수에서의 호출과 반환과 비슷하다는 면에서, 더더욱 
`transfer()`은 구조화되지 않은 goto작동 같습니다. 그것은 당신이 자유롭게 fiber들 사이에서 통제를 전환하도록 
해줍니다. 그것들은 모두 서로의 동료로서 수행됩니다.

<a class="right" href="error-handling.html">Error Handling &rarr;</a>
<a href="classes.html">&larr; Classes</a>
