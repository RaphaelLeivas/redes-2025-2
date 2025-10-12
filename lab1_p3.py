import matplotlib.pyplot as plt

# Load data
orig = [tuple(map(float, line.split())) for line in open("delay_results.txt")]

x1, y1 = zip(*orig)

plt.plot(x1, y1, 'o-', label="Topologia Modificada")

plt.xlabel("Número do Pacote")
plt.ylabel("Atraso Ponto-a-ponto (s)")
plt.title("Atraso Ponto-a-ponto vs. Número do Pacote")
plt.legend()
plt.grid(True)
plt.show()
