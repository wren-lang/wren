function fib(n) {
  if (n < 2) return n;
  return fib(n - 1) + fib(n - 2);
}

var start = process.hrtime();
var i = 0;
for (var i = 0; i < 5; i++) {
  console.log(fib(30));
}
var elapsed = process.hrtime(start);
elapsed = elapsed[0] + elapsed[1] / 1000000000;
console.log("elapsed: " + elapsed);
