#ifndef ArduinoMedianFilter_h
#define ArduinoMedianFilter_h

/*
  Copyright(c) 2024 Rustam Iskenderov

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files(the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions :

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

/*
  High performance and low memory usage sorting algorithm for running median filter.
*/

template <typename T, unsigned int _size>
class MedianFilter
{
public:
  MedianFilter(){};

  unsigned int size() const
  {
    return _size;
  }

  void set(unsigned int idx, T value)
  {
    _buffer[idx] = value;
  };

  T getMedian()
  {
    sort();
    return _buffer[_size / 2];
  };

  T getAverage(unsigned int samples)
  {
    if (_size < samples)
      samples = _size;

    unsigned int start = (_size - samples) / 2;
    unsigned int end = start + samples;

    sort();

    T sum = 0;
    for (unsigned int i = start; i < end; ++i)
    {
      sum += _buffer[i];
    }
    return (sum / samples);
  }

private:
  T _buffer[_size];

  void sort()
  {
    unsigned int gap = _size;
    unsigned int swapped = true;

    while (gap > 1 || swapped == true)
    {
      gap = (gap * 10) / 13;

      if (gap < 1)
        gap = 1;

      swapped = false;

      for (unsigned int i = 0; i < _size - gap; ++i)
      {
        if (_buffer[i] > _buffer[i + gap])
        {
          T tmp = _buffer[i];
          _buffer[i] = _buffer[i + gap];
          _buffer[i + gap] = tmp;
          swapped = true;
        }
      }
    }
  }
};

#endif
