# Survival learning with censored data

**survlearning: A Unified R Interface for Survival Learning with Censored Data**

Pedro Luiz Ramos, Pontificia Universidad Catolica de Chile.

- [Article](survlearning.pdf): methods, software interface, numerical comparisons,
  and a hard-drive reliability application.
- [LaTeX sources](Overleaf_survlearning.zip): a flat Overleaf-ready directory
  containing one TeX file, one bibliography, figures, and the JSS style files.
- [Replication material](Replication_survlearning.zip): R scripts, baseline
  data, results, and the software version used in the numerical study.

The current package is version 0.3.0. It integrates the RF-SRC, Royston-Parmar
and Gehan fitting components while retaining the method identifiers and
prediction contracts. Upstream-derived code is credited in `inst/COPYRIGHTS`.

The accompanying article documents version 0.2.6. The numerical study
uses the supplied version 0.2.4; its software requirements and execution
instructions are in `README_REPLICATION.md` inside the replication archive.
The compatibility comparison reported in the article uses the same fitted
XGBoost engine and the complete training sample.

The application includes 29,058 eligible drives and 718 failures. The predictive
comparison uses 23,246 training drives and 5,812 held-out drives. The historical
random split illustrates the methods; it does not establish prospective validity.

Code is GPL (>= 3). Backblaze is the data source, and its separate data-use
conditions are described in `DATA.md` in the replication archive.
