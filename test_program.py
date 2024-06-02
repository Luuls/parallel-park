"""
    Para que este script avalie corretamente as saídas
    é necessário compilar o programa com a variável do Makefile
    `RELEASE` definida como false.
"""

import subprocess as sub
import re
from queue import SimpleQueue
from concurrent.futures import ProcessPoolExecutor


def test_program() -> None:
    # Estamos considerando que cada parâmetro assume uma de duas classificações de valores:
    # grandes ou pequenos.
    # Isso nos dá um total de 2^3 = 8 combinações possíveis de parâmetros.
    test_cases = [
        '-c 1 -g 1 -t 1',
        '-c 1 -g 1 -t 25',
        '-c 1 -g 30 -t 1',
        '-c 1 -g 30 -t 25',
        '-c 50 -g 1 -t 1',
        '-c 50 -g 1 -t 25',
        '-c 50 -g 30 -t 1',
        '-c 50 -g 30 -t 25',
    ]

    print('Running tests.')

    results = {}
    with ProcessPoolExecutor() as executor:
        future_to_case = {executor.submit(test_case, case): case for case in test_cases}
        for future in future_to_case:
            case = future_to_case[future]
            success, output = future.result()
            results[case] = (success, output)
            print(f"Test {'passed' if success else 'failed'} for case: {case}")

    print('\nSummary of test cases:')
    for case, (success, output) in results.items():
        print(f'Case: {case}')
        print('Result:', 'Passed' if success else 'Failed')
        print(f'Output of the test:\n{output}')
        print("-----\n")

    print('Now running valgrind for default parameters...')
    result = sub.run(
        [
            'valgrind',
            '--leak-check=full',
            '--show-leak-kinds=all',
            '--track-origins=yes',
            './program',
        ], text=True, capture_output=True)

    output = result.stderr
    return_code = result.returncode

    regex = r'^==\d+==\sHEAP SUMMARY:.*(?:\n.*)*'
    match = re.search(regex, output, re.MULTILINE)
    heap_summary = ''
    if match != None:
        heap_summary = match.group(0)

    print(f'Valgrind return code: {return_code}')
    print(f'Valgrind heap summary:\n{heap_summary}')

def test_case(case: str) -> tuple[bool, str]:
    _, c, _, g, _, t = case.split()
    c, g, t = int(c), int(g), int(t)

    result: str = sub.run(['./program', case], text=True, capture_output=True).stdout.replace(r'\n', '\n')

    regex: str = r'\[CASH\] - Turista \[(\d+)\] comprou \[(\d+)\] moedas.'
    matches = re.finditer(regex, result)
    clients_coins: list[int] = [0 for _ in range(c)]
    for match in matches:
        id = int(match.group(1))
        coins = int(match.group(2))
        clients_coins[id - 1] = coins

    regex = r'\[WAITING\] - Turista \[(\d+)\] entrou na fila do portao principal'
    matches = re.finditer(regex, result)
    clients_queue: SimpleQueue[int] = SimpleQueue()
    for match in matches:
        id = int(match.group(1))
        clients_queue.put(id)

    output = 'Testing if queue operations are valid...\n'
    regex = r'\[INFO\] - Turista \[(\d+)\] está aproveitando o parque...'
    matches = re.finditer(regex, result)
    for match in matches:
        if clients_queue.empty():
            return False, output

        if int(match.group(1)) != clients_queue.get():
            return False, output

    output += 'Passed.\n'

    output += 'Testing if clients are spending coins correctly...\n'
    regex = r'\[CASH\] - Turista \[(\d+)\] gastou uma moeda. Restam \[\d+\] moedas.'
    matches = re.finditer(regex, result)
    for match in matches:
        id = int(match.group(1))
        if clients_coins[id - 1] == 0:
            return False, output

        clients_coins[id - 1] -= 1

    if not all([coin == 0 for coin in clients_coins]):
         return False, output

    output += 'Passed.\n'

    return True, output


if __name__ == '__main__':
    test_program()
