package printer_v0

import (
	"fmt"
	"strconv"
	"strings"
)

type Request struct {
	LastId        int
	CompleteSteps int
	X             int
	Y             int
	Z             int
	T             int
	F             int
	Analog        [16]float64
	Read          [32]bool
}

func (r *Request) ToStringSep(sep string) string {
	res := fmt.Sprintf("%v%v", r.LastId, sep)
	res += fmt.Sprintf("%v%v", r.CompleteSteps, sep)
	res += fmt.Sprintf("%v%v", r.X, sep)
	res += fmt.Sprintf("%v%v", r.Y, sep)
	res += fmt.Sprintf("%v%v", r.Z, sep)
	res += fmt.Sprintf("%v%v", r.T, sep)
	res += fmt.Sprintf("%v%v", r.F, sep)
	for i := 0; i < 16; i++ {
		res += fmt.Sprintf("%v%v", r.Analog[i], sep)
	}
	for i := 0; i < 32; i++ {
		if r.Read[i] {
			res += fmt.Sprintf("%v%v", 1, sep)
		} else {
			res += fmt.Sprintf("%v%v", 0, sep)
		}
	}
	return res
}

func (r *Request) FromStringSep(body string, sep string) (err error) {
	data := strings.Split(body, sep)
	if len(data) < 7+16+32 {
		return fmt.Errorf("мало информации %v ожидаем %v", len(data), 7+16+32)
	}

	r.LastId, err = parseInt(data[0])
	if err != nil {
		return err
	}

	r.CompleteSteps, err = parseInt(data[1])
	if err != nil {
		return err
	}

	r.X, err = parseInt(data[2])
	if err != nil {
		return err
	}

	r.Y, err = parseInt(data[3])
	if err != nil {
		return err
	}

	r.Z, err = parseInt(data[4])
	if err != nil {
		return err
	}

	r.T, err = parseInt(data[5])
	if err != nil {
		return err
	}

	r.F, err = parseInt(data[6])
	if err != nil {
		return err
	}

	for i := 0; i < 16; i++ {
		r.Analog[i], err = parseFloat(data[7+i])
		if err != nil {
			return err
		}
	}

	for i := 0; i < 32; i++ {
		r.Read[i], err = parseBool(data[7+16+i])
		if err != nil {
			return err
		}
	}

	return nil
}

func parseInt(s string) (val int, err error) {
	return strconv.Atoi(s)
}
func parseFloat(s string) (val float64, err error) {
	return strconv.ParseFloat(s, 64)
}
func parseBool(s string) (val bool, err error) {
	if s == "1" {
		return true, nil
	} else if s == "0" {
		return false, nil
	}

	return false, fmt.Errorf("допустимы значения 0 или 1, значение $v", s)
}

type Response struct {
	Id    int
	N     int
	Ts    int
	Te    int
	X     int
	Y     int
	Z     int
	T     int
	F     int
	Write [32]bool
}

func (r *Response) ToStringSep(sep string) string {
	res := fmt.Sprintf("%v%v", r.Id, sep)
	res += fmt.Sprintf("%v%v", r.N, sep)
	res += fmt.Sprintf("%v%v", r.Ts, sep)
	res += fmt.Sprintf("%v%v", r.Te, sep)
	res += fmt.Sprintf("%v%v", r.X, sep)
	res += fmt.Sprintf("%v%v", r.Y, sep)
	res += fmt.Sprintf("%v%v", r.Z, sep)
	res += fmt.Sprintf("%v%v", r.T, sep)
	res += fmt.Sprintf("%v%v", r.F, sep)
	for i := 0; i < 32; i++ {
		if r.Write[i] {
			res += fmt.Sprintf("%v%v", 1, sep)
		} else {
			res += fmt.Sprintf("%v%v", 0, sep)
		}
	}
	return res
}
func (r *Response) FromStringSep(body string, sep string) (err error) {
	data := strings.Split(body, sep)
	if len(data) < 9+32 {
		return fmt.Errorf("мало информации %v ожидаем %v", len(data), 9+32)
	}

	r.Id, err = parseInt(data[0])
	if err != nil {
		return err
	}

	r.N, err = parseInt(data[1])
	if err != nil {
		return err
	}

	r.Ts, err = parseInt(data[2])
	if err != nil {
		return err
	}

	r.Te, err = parseInt(data[3])
	if err != nil {
		return err
	}

	r.X, err = parseInt(data[4])
	if err != nil {
		return err
	}

	r.Y, err = parseInt(data[5])
	if err != nil {
		return err
	}

	r.Z, err = parseInt(data[6])
	if err != nil {
		return err
	}

	r.T, err = parseInt(data[7])
	if err != nil {
		return err
	}

	r.F, err = parseInt(data[8])
	if err != nil {
		return err
	}

	for i := 0; i < 32; i++ {
		r.Write[i], err = parseBool(data[9+i])
		if err != nil {
			return err
		}
	}

	return nil
}

func ResponseListToStringSep(respList []*Response, sepRows string, sepCols string) string {
	res := ""
	for i, row := range respList {
		if i > 0 {
			res += sepRows
		}

		res += row.ToStringSep(sepCols)
	}
	return res
}
func ResponseListFromStringSep(body string, sepRows string, sepCols string) (respList []*Response, err error) {
	data := strings.Split(body, sepRows)

	respList = make([]*Response, 0, len(data))

	for _, rowBody := range data {
		var row Response
		err = row.FromStringSep(rowBody, sepCols)
		if err != nil {
			return respList, err
		}
		respList = append(respList, &row)
	}

	return respList, nil
}
