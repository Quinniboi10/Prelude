#pragma once

#include "types.h"
#include "utils.h"

class Board;

class NNUE {
   public:
    alignas(32) array<i16, HL_SIZE * 768> weightsToHL;
    alignas(32) array<i16, HL_SIZE> hiddenLayerBias;
    alignas(32) array<array<i16, HL_SIZE * 2>, OUTPUT_BUCKETS> weightsToOut;
    array<i16, OUTPUT_BUCKETS> outputBias;

    constexpr i16 ReLU(const i16 x) {
        if (x < 0) return 0;
        return x;
    }

    constexpr i16 CReLU(const i16 x) {
        i16 ans = x;
        if (x < 0) ans = 0;
        else if (x > inputQuantizationValue) ans = inputQuantizationValue;
        return ans;
    }

    static int feature(Color perspective, Color color, PieceType piece, Square square) {
        // Constructs a feature from the given perspective for a piece
        // of the given type and color on the given square

        int colorIndex = (perspective == color) ? 0 : 1;
        int pieceIndex = static_cast<int>(piece);
        int squareIndex = (perspective == BLACK) ? flipRank(square) : static_cast<int>(square);

        return colorIndex * 64 * 6 + pieceIndex * 64 + squareIndex;
    }

    constexpr NNUE() {
        weightsToHL.fill(1);

        hiddenLayerBias.fill(0);

        for (auto& w : weightsToOut) w.fill(1);
        outputBias.fill(0);
    }

    void loadNetwork(const std::string& filepath);

    int forwardPass(const Board* board);
};

// Main eval NNUE to be used. Will be loaded during main
extern NNUE nn;