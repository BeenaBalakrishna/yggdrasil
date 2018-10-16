import os
from cis_interface.examples.tests import TestExample


class TestExamplePaper(TestExample):
    r"""Test the Paper example."""

    def __init__(self, *args, **kwargs):
        super(TestExamplePaper, self).__init__(*args, **kwargs)
        self._name = 'paper'

    @property
    def output_dir(self):
        r"""str: Output directory."""
        if self.yamldir is None:  # pragma: debug
            return None
        return os.path.join(self.yamldir, 'Output')

    @property
    def output_files(self):
        r"""list: Output files for the run."""
        outdir = self.output_dir
        filelist = ['root_output.txt',
                    'shoot_output.txt']
        out = [os.path.join(outdir, f) for f in filelist]
        return out

    def check_results(self):
        r"""This should be overridden with checks for the result."""
        pass

    def run_example(self):
        r"""This runs an example in the correct language."""
        if self.output_dir is not None:
            if not os.path.isdir(self.output_dir):
                os.mkdir(self.output_dir)
        super(TestExamplePaper, self).run_example()
