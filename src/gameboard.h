/* ==== Game ==== */
#pragma once

#include "common.h"

typedef struct GameBoard
{
    u16 width, height;

    u16* data        = nullptr;
    u16* target_data = nullptr;

    void init(u16 width, u16 height)
    {
        this->height = height;
        this->width  = width;

        if (this->data != nullptr)
            free(data);

        this->data = (u16*)calloc(height * width, sizeof(u16));

        if (this->target_data != nullptr)
            free(target_data);

        this->target_data = (u16*)calloc(height * width, sizeof(u16));
    }

    u16 getCellTargetValue(u16 x, u16 y) const
    {
        assert(x < width || y < height || data != nullptr);

        return this->target_data[y * width + x];
    }

    void setCellTargetValue(u16 x, u16 y, u16 value)
    {
        assert(x < width || y < height || data != nullptr);

        this->target_data[y * width + x] = value;
    }

    u16 getCellValue(u16 x, u16 y) const
    {
        assert(x < width || y < height || data != nullptr);

        return this->data[y * width + x];
    }

    void setCellValue(u16 x, u16 y, u16 value)
    {
        assert(x < width || y < height || data != nullptr);

        this->data[y * width + x] = value;
    }

    void swap_buffers()
    {
        u16* tmp          = this->data;
        this->data        = this->target_data;
        this->target_data = tmp;
    }

    void copy_buffers(bool data_as_src)
    {
        if (data_as_src)
            memcpy(this->target_data, this->data, this->width * this->height * sizeof(u16));
        else
            memcpy(this->data, this->target_data, this->width * this->height * sizeof(u16));
    }

    void shiftVertical(i8 sign, u16 col, u16 start = 0)
    {
        assert(sign == 1 || sign == -1);

        u16 init_val   = start;
        u16 target_val = (sign == 1) ? this->height - 1 : 0;

        for (u16 j = init_val; j != target_val; j += sign)
        {
            u16 next_val = this->getCellTargetValue(col, j + sign);

            this->setCellTargetValue(col, j, next_val);

            if (j == target_val - sign)
            {
                this->setCellTargetValue(col, j + sign, 0);
            }
        }
    }

    void shiftHorizontal(i8 sign, u16 row, u16 start = 0)
    {
        assert(sign == 1 || sign == -1);

        u16 init_val   = start;
        u16 target_val = (sign == 1) ? this->width - 1 : 0;

        for (u16 i = init_val; i != target_val; i += sign)
        {
            u16 next_val = this->getCellTargetValue(i + sign, row);

            this->setCellTargetValue(i, row, next_val);

            if (i == target_val - sign)
            {
                this->setCellTargetValue(i + sign, row, 0);
            }
        }
    }

    void moveVertical(i8 sign)
    {
        assert(sign == 1 || sign == -1);
        u16 init_val   = (sign == 1) ? 0 : this->height - 1;
        u16 target_val = (sign == 1) ? this->height - 1 : 0;

        for (u16 i = 0; i < this->width; i++)
        {
            for (u16 j = init_val; j != target_val; j += sign)
            {
                u16 curr_val = this->getCellTargetValue(i, j);
                u16 dist     = 1;
                while (curr_val == 0 && dist++ < this->height)
                {
                    shiftVertical(sign, i, j);
                    curr_val = this->getCellTargetValue(i, j);
                }
            }

            for (u16 j = init_val; j != target_val; j += sign)
            {
                u16 curr_val  = this->getCellTargetValue(i, j);
                u16 other_val = this->getCellTargetValue(i, j + sign);

                if (curr_val == other_val)
                {
                    this->setCellTargetValue(i, j, curr_val * 2);
                    this->setCellTargetValue(i, j + sign, 0);
                }
            }

            for (u16 j = init_val; j != target_val; j += sign)
            {
                u16 curr_val = this->getCellTargetValue(i, j);
                u16 dist     = 1;
                while (curr_val == 0 && dist++ < this->height)
                {
                    shiftVertical(sign, i, j);
                    curr_val = this->getCellTargetValue(i, j);
                }
            }
        }
    }

    void moveHorizontal(i8 sign)
    {
        assert(sign == 1 || sign == -1);
        u16 init_val   = (sign == 1) ? 0 : this->width - 1;
        u16 target_val = (sign == 1) ? this->width - 1 : 0;

        for (u16 i = 0; i < this->height; i++)
        {
            for (u16 j = init_val; j != target_val; j += sign)
            {
                u16 curr_val = this->getCellTargetValue(j, i);
                u16 dist     = 1;
                while (curr_val == 0 && dist++ < this->width)
                {
                    shiftHorizontal(sign, i, j);
                    curr_val = this->getCellTargetValue(j, i);
                }
            }

            for (u16 j = init_val; j != target_val; j += sign)
            {
                u16 curr_val  = this->getCellTargetValue(j, i);
                u16 other_val = this->getCellTargetValue(j + sign, i);

                if (curr_val == other_val)
                {
                    this->setCellTargetValue(j, i, curr_val * 2);
                    this->setCellTargetValue(j + sign, i, 0);
                }
            }

            for (u16 j = init_val; j != target_val; j += sign)
            {
                u16 curr_val = this->getCellTargetValue(j, i);
                u16 dist     = 1;
                while (curr_val == 0 && dist++ < this->width)
                {
                    shiftHorizontal(sign, i, j);
                    curr_val = this->getCellTargetValue(j, i);
                }
            }
        }
    }

    void moveUp()
    {
        moveVertical(1);
        addRandomValue();
    }
    void moveDown()
    {
        moveVertical(-1);
        addRandomValue();
    }
    void moveLeft()
    {
        moveHorizontal(1);
        addRandomValue();
    }
    void moveRight()
    {
        moveHorizontal(-1);
        addRandomValue();
    }

    u16 getEmptyCells() const
    {
        u16 count = 0;
        for(u16 i = 0; i < this->width * this->height; i++)
            if(this->data[i] == 0)
                count++;
        return count;
    }

    u16 getMaxValue() const
    {
        u16 max = 0;
        for(u16 i = 0; i < this->width * this->height; i++)
            if(this->data[i] > max)
                max = this->data[i];
        return max;
    }

    bool checkBlock()
    {
        if (getEmptyCells() > 0)
            return false;

        return true;
    }

    void addRandomValue(u16 max_pow = 4)
    {
        const int r   = rand() % max_pow;
        const u16 val = std::pow(2, r);

        u16 x, y;
        do
        {
            x = rand() % this->width;
            y = rand() % this->height;
            if (getEmptyCells() == 0)
                return;
        } while (getCellTargetValue(x, y) != 0);

        setCellTargetValue(x, y, val);
    }
} GameBoard;
