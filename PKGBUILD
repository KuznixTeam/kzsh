pkgname=ksh
pkgver=0.1
pkgrel=1
pkgdesc="Kuznix Shell - Bash-like shell"
arch=('x86_64')
url="https://github.com/KuznixTeam/kzsh"
pkgname=kzsh
license=('GPL3')
depends=('bash')
makedepends=('gcc' 'g++' 'make' 'autoconf' 'automake')
source=("$pkgname-$pkgver.tar.gz")
md5sums=('SKIP')

build() {
  cd "$srcdir/$pkgname-$pkgver"
  ./configure --prefix=/usr
  make
}

package() {
  cd "$srcdir/$pkgname-$pkgver"
  make DESTDIR="$pkgdir" install
  install -Dm644 README.md "$pkgdir/usr/share/doc/$pkgname/README.md"
}
