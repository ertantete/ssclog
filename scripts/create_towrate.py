fname = 'towrate.txt'


def calcrate(i):
    if i < 10:
        return 25.0
    else:
        return 25.0 + (i-10)


def add_ssc_towrate(lines):
    for i in range(100):
        lines.append(f'\t{i*100}\t{calcrate(i):.2f}')


def add_winchrate(lines):
    for i in range(31):
        lines.append(f'\t{i * 100}\t{0:.2f}')


def write_file(lines):
    # print(lines)
    with open(fname, 'w') as f:
        f.write('\n'.join(lines))


def create_towrate_file():
    lines = ['- towplane, tail-number',
             '- (leading tab) release altitude, cost']

    lines.append('SSC Pawnee\tN424BY')
    add_ssc_towrate(lines)

    lines.append('SSC Husky\tN6085S')
    add_ssc_towrate(lines)

    lines.append('Self Launch Motorglider\tNA')
    lines.append('\t0\t0.00')

    lines.append('ESC Winch Launch\tNA')
    add_winchrate(lines)

    write_file(lines)


if __name__ == '__main__':
    create_towrate_file()

