# 🟦 Jogo Geométrico 3D — Agar.io Style

Projeto de **jogo 3D inspirado em Agar.io**, desenvolvido em **C utilizando OpenGL e (free)GLUT**.
O jogador controla um cubo em uma arena fechada, consome pellets e inimigos menores para crescer em massa e deve evitar inimigos maiores, que causam **Game Over instantâneo**.

Este projeto foi criado com foco em **aprendizado prático de programação de jogos**, renderização 3D, matemática vetorial e lógica de tempo real.

---

## 🎮 Gameplay

* Controle de um cubo em arena 3D
* Crescimento progressivo baseado em massa (estilo Agar.io)
* Inimigos com tamanhos e velocidades variadas
* Morte instantânea ao colidir com inimigos maiores
* Avanço de nível conforme a pontuação

---

## ✨ Funcionalidades

* ✅ Renderização 3D com OpenGL
* ✅ Arena completa com piso, paredes, portão e torres
* ✅ Sistema de colisão baseado em raio
* ✅ Crescimento do jogador proporcional ao que é consumido
* ✅ Inimigos móveis com comportamento simples (IA básica)
* ✅ Dash com cooldown
* ✅ Sistema de partículas (coleta, dash e inimigos derrotados)
* ✅ Movimentação fluida com aceleração e desaceleração
* ✅ Dois modos de controle: Mouse (Agar.io style) ou Teclado (WASD)
* ✅ HUD com score, massa, nível e modo de controle
* ✅ Menu inicial e tela de controles
* ✅ Sistema de pausa e reinício

---

## 🕹️ Controles

### Movimento

* **Mouse** → Movimento em direção ao cursor (modo padrão)
* **W / A / S / D** ou **Setas** → Movimento no plano XZ
* **M** → Alterna entre Mouse e Teclado

### Ações

* **Espaço** → Dash curto na direção do movimento
* **P** → Pausar / Despausar
* **R** → Reiniciar o jogo
* **ESC** → Sair do jogo

---

## 🧱 Tecnologias Utilizadas

* **Linguagem:** C / C++ (estilo C)
* **Gráficos:** OpenGL
* **Janela/Input:** GLUT / freeGLUT
* **Matemática:** Vetores 3D, colisão por distância
* **Paradigma:** Programação procedural

---

## ⚙️ Compilação e Execução

### 📌 Requisitos

* OpenGL
* GLUT ou freeGLUT
* Compilador C/C++ (GCC, Clang ou MSVC)

---

### 🐧 Linux

```bash
g++ main.cpp -o jogo \
    -lGL -lGLU -lglut
./jogo
```

Se estiver usando **freeGLUT**:

```bash
g++ main.cpp -o jogo \
    -lGL -lGLU -lfreeglut
```

---

### 🪟 Windows (MinGW)

```bash
g++ main.cpp -o jogo.exe \
    -lfreeglut -lopengl32 -lglu32
```

> Certifique-se de que as DLLs do freeGLUT estejam no mesmo diretório ou no PATH.

---

### 🍎 macOS

```bash
clang++ main.cpp -framework OpenGL -framework GLUT -o jogo
```

---

## 🧠 Estrutura Geral do Código

* **Renderização**: Cubos, pirâmides, partículas e iluminação
* **Cenário**: Piso quadriculado, paredes, portão e torres
* **Entidades**:

  * Jogador
  * Pellets (comida)
  * Inimigos (cubos vermelhos)
* **Lógica do jogo**:

  * Colisão
  * Crescimento
  * Progressão de níveis
  * Sistema de partículas
* **Interface**:

  * HUD
  * Menu inicial
  * Tela de controles

---

## 📈 Possíveis Melhorias Futuras

* 🔲 Som e trilha sonora
* 🔲 IA mais avançada para inimigos
* 🔲 Power-ups
* 🔲 Multiplayer local ou online
* 🔲 Sistema de ranking
* 🔲 Melhorias visuais (sombras, shaders)

---

## 📚 Objetivo do Projeto

Este projeto foi desenvolvido com fins **educacionais**, visando:

* Aprimorar conhecimentos em OpenGL
* Praticar lógica de jogos em tempo real
* Trabalhar matemática aplicada a jogos
* Explorar arquitetura básica de game loop

---

## 👤 Autor

Desenvolvido por **Beicu**
Projeto pessoal para estudos em **programação de jogos e computação gráfica**.

---

## 📜 Licença

Este projeto é livre para estudo e modificação para fins educacionais.
