cc_library(name = 'linear',
          srcs = [
                  'linear.cc',
                 ],
          deps = [
                  ':blas',
                 ],
          warning = 'no',
          )
          
cc_library(name = 'blas',
          srcs = [
                  'ddot.cc',
                  'daxpy.cc',
                  'dscal.cc',
                  'tron.cc',
                  'dnrm2.cc',
                 ],
          )

cc_binary(name = 'predict',
          srcs = [
                  'predict.cc',
                 ],
          deps = [
                  ':linear',
                 ],
          )
          
cc_binary(name = 'train',
          srcs = [
                  'train.cc',
                 ],
          deps = [
                  ':linear',
                 ],
          )
