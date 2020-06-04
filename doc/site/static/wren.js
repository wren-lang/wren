window.onload = function() {
  var blocks = document.querySelectorAll('pre.snippet')
  blocks.forEach((element) => {
    var lang = 'lua'
    var input_lang = element.getAttribute('data-lang')
    if(input_lang) lang = input_lang
    var code = document.createElement('code');
    code.setAttribute('class', ' language-'+lang);
    code.innerHTML = element.innerHTML;
    element.innerHTML = '';
    element.append(code)
  });
  Prism.highlightAll();


  var try_code = document.querySelector("#try-code")
  if(try_code) {
    var output = document.querySelector("#try-output")
    Module.print = function(text) { output.innerText += text + "\n"; }
    Module.printErr = function(text) { output.innerText += text + "\n"; }

    var run = document.querySelector("#try-run")
    var hello = document.querySelector("#try-hello")
    var fractal = document.querySelector("#try-fractal")
    var loop = document.querySelector("#try-loop")
    var compile = Module.cwrap('wren_compile', 'number', ['string'])

    var set_input = function(content) {
      output.innerText = '...';
      var input = document.querySelector('.prism-live code.language-lua');
      input.innerHTML = content;
      Prism.highlightElement(input)
    }

    run.onclick = function(e) {
      console.log("run")
      output.setAttribute('ready', '')
      output.innerText = ''
      var input = document.querySelector('.prism-live code.language-lua');
      var result = compile(input.innerText)
      var message = "All good!"
      if(result == 1) { //WREN_RESULT_COMPILE_ERROR
        message = "Compile error!"
      } else if(result == 2) { //WREN_RESULT_RUNTIME_ERROR
        message = "Runtime error!"
      }
      // Module.print('\n\n---\n' + message)
      console.log(result);
    }

    hello.onclick = function(e) {
      set_input('System.print("hello wren")')
    }

    loop.onclick = function(e) {
      set_input(`for (i in 1..10) System.print("Counting up %(i)")`);
    }

    fractal.onclick = function(e) {
      set_input(`for (yPixel in 0...24) {
  var y = yPixel / 12 - 1
  for (xPixel in 0...80) {
    var x = xPixel / 30 - 2
    var x0 = x
    var y0 = y
    var iter = 0
    while (iter < 11 && x0 * x0 + y0 * y0 <= 4) {
      var x1 = (x0 * x0) - (y0 * y0) + x
      var y1 = 2 * x0 * y0 + y
      x0 = x1
      y0 = y1
      iter = iter + 1
    }
    System.write(" .-:;+=xX$& "[iter])
  }
  System.print("")
}`);
    }

  }
}
