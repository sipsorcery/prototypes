extern crate num;
extern crate rustfft;

use num::complex::Complex;
use rustfft::FFT;

const FFT_SIZE: usize = 1024;

fn main() {
    println!("Rust Fft Test");

    let mut n = FFT_SIZE;
    if n % 2 == 0 {
        n -= 1;
    }

    println!("n={}", n);

    let analytic = make_analytic(n, FFT_SIZE);

    // println!("Analytic series:");
    // for i in 0..FFT_SIZE  {
    //     println!("{}: {} {}", i, analytic[i], analytic[i].norm());
    // }

    let mut fft = FFT::new(FFT_SIZE, false);
    let mut ifft = FFT::new(FFT_SIZE, true);
    let mut time_series = vec![Complex::new(0.0f32, 0.0); FFT_SIZE];
    let mut freq_series = vec![Complex::new(0.0f32, 0.0); FFT_SIZE];
    let mut phase_shifted = vec![Complex::new(0.0f32, 0.0); FFT_SIZE];

    for i in 0..FFT_SIZE {
        let re: f32 = (2.0 * std::f64::consts::PI * 3.0 * (i as f64 / FFT_SIZE as f64)).sin() as f32;
        time_series[i].re = re;
    }

    fft.process(&time_series[..], &mut freq_series[..]);

    // println!("Frequency series:");
    // for i in 0..FFT_SIZE  {
    //     println!("{}: {} {}", i, freq_series[i], freq_series[i].norm());
    // }

    for (x, y) in analytic.iter().zip(freq_series.iter_mut()) {
        *y = *x * *y;
    }

    // println!("Modified Frequency series:");
    // for i in 0..FFT_SIZE  {
    //     println!("{}: {} {}", i, freq_series[i], freq_series[i].norm());
    // }

    ifft.process(&freq_series[..], &mut phase_shifted[..]);

    // println!("Recovered time series:");
    // for i in 0..FFT_SIZE  {
    //     println!("{}: {} {}", i, recovered_time_series[i], recovered_time_series[i].norm());
    //}

    let scale = FFT_SIZE as f32;
    let mut count: i32 = 0;
    for(x) in phase_shifted.iter() {
        let xSample = x.re / scale;
        let ySample = x.im / scale;
        println!("{}: {} {}", count, xSample, ySample);
        count += 1;
    }
}

// FIR analytical signal transform of length n with zero padding to be length m
// real part removes DC and nyquist, imaginary part phase shifts by 90
// should act as bandpass (remove all negative frequencies + DC & nyquist)
fn make_analytic(n: usize, m: usize) -> Vec<Complex<f32>> {
    use ::std::f32::consts::PI;
    assert_eq!(n % 2, 1, "n should be odd");
    assert!(n <= m, "n should be less than or equal to m");

    println!("make_analytic, n={}, m={}.", n, m);

    // let a = 2.0 / n as f32;
    let mut fft = FFT::new(m, false);

    let mut impulse = vec![Complex::new(0.0, 0.0); m];
    let mut freqs = impulse.clone();

    let mid = (n - 1) / 2;

    impulse[mid].re = 1.0;
    let re = -1.0 / (mid - 1) as f32;
    for i in 1..mid+1 {
        if i % 2 == 0 {
            impulse[mid + i].re = re;
            impulse[mid - i].re = re;
        } else {
            let im = 2.0 / PI / i as f32;
            impulse[mid + i].im = im;
            impulse[mid - i].im = -im;
        }
        // hamming window
        let k = 0.53836 + 0.46164 * (i as f32 * PI / (mid + 1) as f32).cos();
        impulse[mid + i] = impulse[mid + i].scale(k);
        impulse[mid - i] = impulse[mid - i].scale(k);
    } 

    fft.process(&impulse, &mut freqs);
    freqs
}

