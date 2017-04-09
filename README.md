# TemplMandel

![Almond](https://raw.githubusercontent.com/Fytch/TemplMandel/master/assets/almond.png)

## Compilation time

Results with Clang 4.0.0 on a i7-6800k:

- 16x16: 14s and 800MB RAM
- 32x32: 1m02s and 3.2GB RAM
- 64x64: 4m34s and 13.6GB RAM

## Difficulties I've encountered

- Detecting multiplication overflows with compile-time constants. The trick is to look at the highest 1-bit of every factor and add up their positions. After that, check if the resulting sum is in range (i.e. ≤63 for signed 64-bit integers). Put another way, the addends' binary logarithms must be less than or equal to the number of bits available: ![log2](https://raw.githubusercontent.com/Fytch/TemplMandel/master/assets/log2.png) This trick works because the (binary) logarithm constitutes a group isomorphism ![isomorphism](https://raw.githubusercontent.com/Fytch/TemplMandel/master/assets/isomorphism.png) so instead of comparing the product, we only look at the sum of the logarithms.
- Filling a 2D array with TMP-generated values is a bit more difficult compared to 1D arrays because the simple 1D `discard( ( data[ i ] = gen< i >, 0 )... )` does not generalize to `discard( ( data[ x ][ y ] = gen< x, y >, 0 )... )` as wished; it would only fill the diagonal. My solution was to use a 1D array and convert the indices to 2D: `discard( ( data[ i ] = gen< i % width, i / width >, 0 )... )`

## ...but why?

For the lulz and the keks.
