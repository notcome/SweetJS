func fac(_ n: Int) -> Int {
  if n == 1 {
    return 1
  } else {
    return n * fac(n - 1)
  }
}

func fib(_ n: Int) -> Int {
  var a = 1
  var b = 1
  var i = 0
  repeat {
    let c = a + b
    a = b
    b = c
    i += 1
  } while i < n
  return a
}

